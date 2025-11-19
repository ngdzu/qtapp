# Useful Docker Commands for Qt Learning Course

## Building Images

### Build the shared Qt base image
```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
```
**What it does**: Builds only the `qt-dev-env` stage from the root Dockerfile and tags it as `qtapp-qt-dev-env:latest`. This creates the shared base image that all lessons will reuse.

**Sample output**:
```
[+] Building 46.8s (10/10) FINISHED
 => [qt-dev-env 1/3] RUN apt-get update && apt-get install -y qt6-base-dev...
 => [qt-dev-env 2/3] RUN ln -snf /usr/share/zoneinfo/America/New_York...
 => [qt-dev-env 3/3] WORKDIR /workspace
 => exporting to image
 => => naming to docker.io/library/qtapp-qt-dev-env:latest
```

### Build a lesson image
```bash
cd 01-qt-setup
docker build -t qt-lesson-01 .
```
**What it does**: Builds the lesson Dockerfile, which uses the shared base image to compile and package the lesson executable.

**Sample output**:
```
[+] Building 1.4s (11/11) FINISHED
 => [builder 1/4] FROM docker.io/library/qtapp-qt-dev-env:latest
 => [builder 3/4] COPY CMakeLists.txt main.cpp ./
 => [builder 4/4] RUN mkdir build && cd build && cmake .. && cmake --build .
 => [runtime 3/3] COPY --from=builder /opt/lesson01/qt-setup .
 => => naming to docker.io/library/qt-lesson-01:latest
```

---

## Running Containers

### Run a lesson (basic)
```bash
docker run --rm qt-lesson-01
```
**What it does**: Runs the lesson container and removes it after exit (`--rm`). Will fail without display setup for GUI apps.

**Sample output**:
```
qt.qpa.xcb: could not connect to display
This application failed to start because no Qt platform plugin could be initialized.
```

### Run with X11 display (macOS with XQuartz)
```bash
xhost + 127.0.0.1
docker run --rm -e DISPLAY=host.docker.internal:0 qt-lesson-01
```
**What it does**: 
1. `xhost + 127.0.0.1` allows Docker to connect to your X11 server
2. `-e DISPLAY=host.docker.internal:0` sets the DISPLAY environment variable to point to the host's X11 server
3. Runs the GUI application, which will appear on your screen

### Run with X11 display (Linux)
```bash
xhost +local:docker
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix qt-lesson-01
xhost -local:docker
```
**What it does**: 
1. `xhost +local:docker` grants Docker access to X11
2. `-v /tmp/.X11-unix:/tmp/.X11-unix` mounts the X11 socket into the container
3. `-e DISPLAY=$DISPLAY` passes your display variable
4. Final `xhost -local:docker` revokes access after testing (security best practice)

### Run a command inside a container
```bash
docker run --rm qt-lesson-01 ls -lh /opt/lesson01/qt-setup
```
**What it does**: Runs a specific command (`ls -lh`) instead of the default CMD, then removes the container.

**Sample output**:
```
-rwxr-xr-x 1 root root 15K Nov 19 13:12 /opt/lesson01/qt-setup
```

### Check library linkage
```bash
docker run --rm qt-lesson-01 ldd /opt/lesson01/qt-setup | head -10
```
**What it does**: Shows which shared libraries the executable is linked against (first 10 lines).

**Sample output**:
```
linux-vdso.so.1 (0x0000ffffb2f43000)
libQt6Widgets.so.6 => /lib/aarch64-linux-gnu/libQt6Widgets.so.6 (0x0000ffffb2600000)
libQt6Core.so.6 => /lib/aarch64-linux-gnu/libQt6Core.so.6 (0x0000ffffb2000000)
libstdc++.so.6 => /lib/aarch64-linux-gnu/libstdc++.so.6 (0x0000ffffb1c00000)
libgcc_s.so.1 => /lib/aarch64-linux-gnu/libgcc_s.so.1 (0x0000ffffb2ec0000)
libc.so.6 => /lib/aarch64-linux-gnu/libc.so.6 (0x0000ffffb1e50000)
libQt6Gui.so.6 => /lib/aarch64-linux-gnu/libQt6Gui.so.6 (0x0000ffffb1400000)
libm.so.6 => /lib/aarch64-linux-gnu/libm.so.6 (0x0000ffffb2e20000)
```

---

## Inspecting Images and Containers

### List all images
```bash
docker images
```
**What it does**: Shows all Docker images on your system with their size, tag, and ID.

**Sample output**:
```
REPOSITORY           TAG       IMAGE ID       CREATED          SIZE
qt-lesson-01         latest    b765aedcc4be   5 minutes ago    1.34GB
qtapp-qt-dev-env     latest    b04365f96b28   8 hours ago      1.34GB
qtapp-app            latest    7dad9cf327d9   8 hours ago      1.34GB
```

### List images with filter
```bash
docker images | grep -E "qt-lesson-01|qtapp-qt-dev-env|qtapp-app"
```
**What it does**: Filters the image list to show only Qt-related images.

**Sample output**:
```
qt-lesson-01         latest    b765aedcc4be   5 minutes ago    1.34GB
qtapp-qt-dev-env     latest    b04365f96b28   8 hours ago      1.34GB
qtapp-app            latest    7dad9cf327d9   8 hours ago      1.34GB
```

### Check disk usage
```bash
docker system df
```
**What it does**: Shows Docker disk usage summary (images, containers, volumes, build cache).

**Sample output**:
```
TYPE            TOTAL     ACTIVE    SIZE      RECLAIMABLE
Images          15        3         5.2GB     3.8GB (73%)
Containers      5         1         16.4kB    0B (0%)
Local Volumes   2         0         100MB     100MB (100%)
Build Cache     45        0         1.2GB     1.2GB
```

### Check detailed disk usage with layer sharing
```bash
docker system df -v | grep qt
```
**What it does**: Shows verbose disk usage including SHARED SIZE and UNIQUE SIZE for each image.

**Sample output**:
```
qt-lesson-01         1.34GB    987.7MB    351.8MB    0
qtapp-qt-dev-env     1.34GB    987.7MB    351.8MB    0
qtapp-app            1.34GB    987.7MB    351.8MB    0
```
- **SIZE**: Total image size (including shared layers)
- **SHARED SIZE**: Layers shared with other images (~987 MB Qt base)
- **UNIQUE SIZE**: Actual disk space used by this image alone (~352 MB)

### List running containers
```bash
docker ps
```
**What it does**: Shows currently running containers.

**Sample output**:
```
CONTAINER ID   IMAGE           COMMAND        CREATED         STATUS         NAMES
77872fa34a40   qtapp-app-dev   "/opt/qtapp/d0"   2 hours ago   Up 2 hours   qtapp-app-dev-1
```

### List all containers (including stopped)
```bash
docker ps -a
```
**What it does**: Shows all containers, including those that have exited.

**Sample output**:
```
CONTAINER ID   IMAGE           COMMAND           CREATED        STATUS                    NAMES
77872fa34a40   qtapp-app-dev   "/opt/qtapp/d0"   2 hours ago    Exited (0) 1 hour ago    qtapp-app-dev-1
```

---

## Cleanup Commands

### Remove a specific image
```bash
docker rmi qt-lesson-01
```
**What it does**: Removes the specified image by name or ID.

**Sample output**:
```
Untagged: qt-lesson-01:latest
Deleted: sha256:b765aedcc4be...
Deleted: sha256:cc2939ab9ed4...
```

### Remove all lesson images
```bash
docker images | grep qt-lesson | awk '{print $3}' | xargs docker rmi
```
**What it does**: Finds all images with "qt-lesson" in the name and removes them.

### Remove unused images
```bash
docker image prune
```
**What it does**: Removes dangling images (untagged images that aren't used by any container).

**Sample output**:
```
WARNING! This will remove all dangling images.
Are you sure you want to continue? [y/N] y
Deleted Images:
deleted: sha256:abc123...
Total reclaimed space: 450MB
```

### Remove all unused images, containers, and build cache
```bash
docker system prune -a
```
**What it does**: Deep clean - removes all unused containers, networks, images (both dangling and unreferenced), and build cache.

**Sample output**:
```
WARNING! This will remove:
  - all stopped containers
  - all networks not used by at least one container
  - all images without at least one container associated to them
  - all build cache

Are you sure you want to continue? [y/N] y
Total reclaimed space: 2.1GB
```

### Remove stopped containers
```bash
docker container prune
```
**What it does**: Removes all stopped containers.

---

## Debugging and Development

### Interactive shell in a running container
```bash
docker run --rm -it qt-lesson-01 bash
```
**What it does**: 
- `-it` makes it interactive with a TTY
- `bash` overrides the default CMD
- Gives you a shell inside the container for debugging

**Sample output**:
```
root@abc123:/opt/lesson01# ls
qt-setup
root@abc123:/opt/lesson01# ./qt-setup
qt.qpa.xcb: could not connect to display
```

### Execute command in running container
```bash
docker exec -it <container-id> bash
```
**What it does**: Opens a shell in an already-running container (useful for debugging long-running services).

### Inspect image details
```bash
docker inspect qt-lesson-01
```
**What it does**: Shows detailed JSON information about the image (layers, environment variables, CMD, etc.).

### View image history/layers
```bash
docker history qt-lesson-01
```
**What it does**: Shows all layers in the image and their sizes.

**Sample output**:
```
IMAGE          CREATED         CREATED BY                                      SIZE
b765aedcc4be   10 minutes ago  CMD ["./qt-setup"]                              0B
cc2939ab9ed4   10 minutes ago  COPY /opt/lesson01/qt-setup . # buildkit        15kB
93de49491ebd   8 hours ago     WORKDIR /workspace                              0B
ec4a194c8b14   8 hours ago     RUN apt-get install -y qt6-base-dev...         987MB
```

---

## Build Optimization

### Build with no cache
```bash
docker build --no-cache -t qt-lesson-01 .
```
**What it does**: Forces a complete rebuild without using any cached layers (useful when troubleshooting).

### Build and show verbose output
```bash
docker build --progress=plain -t qt-lesson-01 .
```
**What it does**: Shows detailed build output instead of the condensed view (useful for debugging build failures).

### Build a specific stage
```bash
docker build --target builder -t qt-lesson-01-builder .
```
**What it does**: Builds only up to the specified stage in a multi-stage Dockerfile (useful for debugging intermediate stages).

---

## Docker Compose Commands

### Start services
```bash
docker compose up
```
**What it does**: Builds (if needed) and starts all services defined in `docker-compose.yml`.

### Start in detached mode
```bash
docker compose up -d
```
**What it does**: Starts services in the background.

### Stop services
```bash
docker compose down
```
**What it does**: Stops and removes containers, networks created by `up`.

### View logs
```bash
docker compose logs -f app
```
**What it does**: Shows logs for the `app` service and follows (`-f`) for real-time updates.

### Rebuild and restart
```bash
docker compose up --build
```
**What it does**: Forces rebuild of images before starting services.

---

## Tips and Best Practices

### Always clean up test containers
Use `--rm` flag when running test containers:
```bash
docker run --rm qt-lesson-01
```
This automatically removes the container after it exits.

### Check actual disk usage regularly
```bash
docker system df -v
```
The UNIQUE SIZE column shows real disk consumption per image.

### Tag images meaningfully
```bash
docker build -t qt-lesson-01:v1.0 .
docker build -t qt-lesson-01:latest .
```
Use version tags for tracking different builds.

### Export/Import images
```bash
# Save image to file
docker save -o qt-lesson-01.tar qt-lesson-01:latest

# Load image from file
docker load -i qt-lesson-01.tar
```
Useful for sharing images without a registry.
