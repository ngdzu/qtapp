
# qtapp

This repository contains a small Qt application and a Docker setup to run it. Important: complete the OS-specific X server setup before starting the container so the GUI can be displayed.

## Prerequisites

### Docker
- Install Docker Desktop: https://www.docker.com/products/docker-desktop/

### X server (required for GUI)
- On macOS (XQuartz):
	- Install and open XQuartz: https://www.xquartz.org/
	- In XQuartz -> Preferences -> Security: enable **Allow connections from network clients**.
	- In a host terminal allow local connections from Docker containers (recommended instead of fully open):

		```bash
		xhost + 127.0.0.1
		```

	- Note: avoid `xhost +` because it allows all hosts to connect (insecure).

- On Windows: install and run an X server such as VcXsrv: https://github.com/marchaesen/vcxsrv/releases

Do these steps before running `docker compose up` so the container can connect to your host X server.

## Run (docker-compose)

**Quick answer — commands to run:**

- Build and run (foreground):

	```bash
	docker compose up --build
	```

- Build and run detached (background):

	```bash
	docker compose up --build -d
	```

- Stop and remove containers/networks:

	```bash
	docker compose down
	```

If you are using the legacy Compose binary use `docker-compose` instead of `docker compose`.

## Developing locally (recommended)

For iterative development on your Mac (edit source locally, run inside container), use the `app-dev` service defined in `docker-compose.override.yml`.

- The override service bind-mounts your project into the container so edits are visible immediately.
- It builds from the mounted source on container start and runs the `d0` binary.

Commands:

```bash
# Build+run the development service in foreground (rebuilds at start):
docker compose up --build app-dev

# Run detached:
docker compose up --build -d app-dev

# Stop the dev service:
docker compose stop app-dev

# If you prefer manual control, open a shell inside the dev container and run builds:
docker compose exec app-dev bash
cd build
cmake ..
make -j$(nproc)
./d0
```

Notes:

- If you keep a bind-mount `.:/workspace` the image-built binary (in `/opt/qtapp`) will be shadowed by the mounted source tree. That is intentional for development so you can build/run the code you edit locally.
- To keep a reproducible runtime image (for CI or demo), use the `app` service (no source mount) which runs the binary built into the image.

### Allowing X connections from containers (helper)

To avoid running `xhost +` (insecure) you can add only the container IP(s) that need access. A helper script is provided at `scripts/xhost-allow-for-compose.sh`.

Examples:

```bash
# Allow containers for the 'app-dev' service to connect to your X server
./scripts/xhost-allow-for-compose.sh app-dev allow

# Revoke the permission later
./scripts/xhost-allow-for-compose.sh app-dev revoke
```

The script uses `docker compose ps -q <service>` to discover running container IDs, inspects their IPs, and runs `xhost +<ip>` or `xhost -<ip>`. It requires `docker` and `xhost` available on your host. If you prefer a single-line, you can also do:

```bash
# one-liner: find first container IP for service and add it to xhost
xhost +$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $(docker compose ps -q app-dev))
```

Be sure to remove the permission when finished: `./scripts/xhost-allow-for-compose.sh app-dev revoke` or `xhost - <ip>`.

## Development workflow (recommended)

This is a concise, repeatable workflow for editing the code locally and running the app inside the development container.

1. Edit code on your host

	- Use your editor/IDE on macOS to modify files in the repository.

2. Allow X connections for the running dev container (one-time per run)

```bash
# Start the dev container first (detached or foreground). If not running, start it:
docker compose up --build -d app-dev

# Add the container IP(s) to X server access (safer than xhost +)
./scripts/xhost-allow-for-compose.sh app-dev allow
```

3. Build and run the code inside the dev container

Option A — let the `app-dev` service build and run at start (convenient):

```bash
docker compose up --build app-dev       # foreground (rebuilds at start)
docker compose up --build -d app-dev    # detached
```

Option B — manual iterative workflow (fast rebuilds):

```bash
docker compose exec app-dev bash         # open shell in running dev container
mkdir -p build && cd build
cmake ..
make -j$(nproc)
./d0                                   # run the built binary
```

4. Iterate

- Edit source on the host, then re-run `make -j$(nproc)` inside the container (Option B), or restart `app-dev` (Option A) to rebuild.
- For quick UI tests, you can run just `./d0` after building.

5. When ready to test the final image (production-like)

```bash
docker compose up --build app           # builds image with compiled binary embedded
docker compose up --build -d app        # run detached
```

6. Cleanup Xhost permission when finished

```bash
./scripts/xhost-allow-for-compose.sh app-dev revoke
# or explicitly: xhost - <ip>
```

7. Commit and push

- Commit the changes you made locally and push to your repo as usual:

```bash
git add -A
git commit -m "Describe change"
git push
```

Notes & tips

- The `app-dev` service uses a bind mount (`.:/workspace`) so edits on the host appear immediately in the container.
- The `app` service is intended for reproducible runs and CI; it uses the image-built binary, so you must rebuild the image to pick up source changes.
- Use the `xhost` helper script to restrict X access to only the container IP(s) instead of `xhost +`.
- If X windows fail to appear, verify XQuartz preferences (Allow network clients) and check `xhost` output and container logs.

**Service name:** The Compose file defines a single service named `app` (see `docker-compose.yml`). To run only that service:

```bash
docker compose up --build app


**View logs:**

```bash
docker compose logs -f app
```

**Run a shell inside the container:**

```bash
docker compose run --rm app /bin/bash
# or, if bash isn't available:
docker compose run --rm app sh
```

## Notes and hints
- The `docker-compose.yml` builds the image from the local `Dockerfile` and mounts the project into the container at `/workspace`.
- The `ports` section in the Compose file is commented out, so no host ports are published by default. If the app needs a port exposed, uncomment and set the mapping (for example `- "8080:8080"`).
- The Compose file sets `DISPLAY=host.docker.internal:0` and mounts `/tmp/.X11-unix` to enable X11 forwarding from macOS. The `DISPLAY` value and X server configuration must match the host setup.

## Troubleshooting
- If the GUI doesn't appear, ensure XQuartz (or your X server) is running, network clients are allowed, and `xhost` permissions include the container host (see above).
- If you need to expose network ports (HTTP, etc.), add a `ports:` mapping under the `app` service in `docker-compose.yml` and re-run `docker compose up --build`.

## More
- The repository mounts your workspace into the container, so changes made on your host are visible immediately inside the container.

If you'd like, I can add a small `Makefile` or convenience scripts (e.g. `make up`, `make down`) to simplify these commands.

