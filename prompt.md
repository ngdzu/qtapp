Qt learning course — course-generation prompt

Goal
Create a lesson-based Qt learning course where each lesson is a self-contained folder that can be built and run inside a container. Output should be a filesystem-style course: a top-level directory containing per-lesson folders. Each lesson must be focused and small enough that a single executable demonstrates the concept.

Required outputs per lesson folder
- `lesson.md`: clear pedagogical write-up (300–600 words) explaining the concept, learning goals, and example walkthrough. Include 2–3 short code snippets and a short “expected output / run result” section.
- `quiz.md`: 5–10 questions explicitly mapped to the lesson’s learning goals. Include a mix of:
	- 2–3 conceptual questions ("why" / trade‑offs / when to use a feature),
	- 2–4 practical/code questions ("what does this code do?", "fill in the missing line", "spot the bug"),
	- and 1–2 reflection questions ("how would you use this in your own project?").
	Clearly number all questions and, where applicable, show short Qt/C++ snippets instead of only prose.
- `answers.md`: repeat each question followed by its answer. For every answer, add:
	- a one‑sentence direct answer,
	- a 1–3 sentence explanation that references specific parts of the lesson or code,
	- and, for code questions, the corrected / complete code snippet.
	If a question is common‑pitfall oriented, briefly describe the pitfall and how to avoid it.
- `CMakeLists.txt`: minimal CMake setup to configure, build, and produce a single executable for the lesson.
- Source files: `.h` and `.cpp` (or `.qml` / `.cpp` when Qt Quick is used). Only one `main()` across the lesson files — a single executable entrypoint.
- `README.md`: per-lesson instructions that explain how to build and run the lesson project. At minimum, include:
	- the exact commands to configure and build the lesson (for example: `mkdir build && cd build && cmake .. && cmake --build .`),
	- the exact command to run the resulting executable inside the container,
	- a short note about any required environment variables, ports, or Docker options specific to this lesson,
	- for GUI applications, X11 setup instructions: mention that XQuartz (macOS) or an X server must be running, and include a note to run the `../scripts/xhost-allow-for-compose.sh` helper script at least once before running the container to grant X11 access. Show both macOS and Linux examples.
	- **IMPORTANT**: Do NOT add comments inside code blocks. Write all explanations and instructions as descriptive text before each code snippet to keep commands copy-pasteable.

Container / build constraints
- Assume lessons will be built and run inside a container. Each lesson that requires additional libraries (Qt Quick, Qt Multimedia, SQL drivers, OpenGL, etc.) must include its own `Dockerfile` (or `docker-compose.yml`) inside the lesson folder that installs those dependencies. Do not rely on a monolithic root Dockerfile.
- **Shared base image pattern**: All lessons should use `FROM qtapp-qt-dev-env:latest` as their base to avoid duplicating Qt installation across multiple images. The base image is built from the root Dockerfile with `docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .` and provides Qt 6, CMake, and all common build tools.
- Build/run commands expected in `README.md`: `mkdir build && cd build && cmake .. && cmake --build .` then run `./<lesson-executable>` (or `./run.sh` wrapper if needed). Tests are optional.
- Target Qt version: default to Qt 6.x (explicitly state exact Qt module dependencies per lesson). If a lesson requires Qt 5, document why and include compatible Dockerfile.
 - Keep container images lean: prefer a shared base image pattern (e.g., a common Qt runtime/base stage) and small per-lesson layers rather than many completely separate heavy images.
 - Avoid storing or generating large binary assets or many large images inside the repo or image; use small placeholder assets when needed and document where real assets would go.
 - When writing Dockerfiles, favor multi-stage builds, cleaning build artifacts where appropriate, and reusing layers to minimize overall disk usage.

Lesson design constraints
- Keep lessons focused: 1 main concept per lesson (e.g., signals & slots, layouts, model/view, QML basics). Avoid mixing multiple major topics into one lesson.
- Small, copy-paste-able examples that compile without modification in the lesson container.
- Minimal external dependencies beyond Qt modules. If additional libraries are used, include install steps in the lesson `Dockerfile`.

Quality & style guidance for generated content
- Use clear, concise language targeted at C++ developers new to Qt.
- Provide small, idiomatic Qt code examples that follow basic C++ style (no unnecessary complexity).
- The `lesson.md` must contain a short “Try it” exercise and expected learning outcomes.

Meta & output format
- Produce a top-level plan (table of contents) listing lesson folder names, brief descriptions, difficulty and needed Qt modules.
- For each lesson, generate the required files described above.
- Ensure `CMakeLists.txt` is minimal and cross-platform-friendly (use standard CMake, `find_package(Qt6 COMPONENTS Widgets REQUIRED)` etc.).
- Include a short note on how to run the lesson inside Docker for each lesson's `README.md`.

Checklist for each generated lesson (must be satisfied before finishing a lesson):
1. Folder contains `lesson.md`, `quiz.md`, `answers.md`, `CMakeLists.txt`, `README.md`, source files, and if needed, a `Dockerfile`.
2. Code compiles and links with the specified Qt modules inside the included `Dockerfile`.
3. Exactly one executable with `main()` demonstrates the lesson concept.
4. `lesson.md` length is 300–600 words and includes learning goals, steps, and sample output.

If you produce a full course scaffold, output it as a list of lesson folders and the files for each lesson. Also provide a final `table_of_contents.md` summarizing all lessons.

When appropriate, prefer Qt Widgets lessons first (core concepts), then Qt Quick / QML lessons, then advanced topics (model/view, threads, networking, multimedia, OpenGL, packaging and CI). Keep the entire course practical and runnable in Docker.

End of prompt.




