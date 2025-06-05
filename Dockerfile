# Stage 1: Build Stage
# Use an official GCC image to build the C++ application
FROM gcc:14.2.0 AS build

# Install required libraries and dependencies for building the app
RUN apt-get update && apt-get install -y \
    libsqlite3-dev \
    cmake \
    make \
    wget \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory for the build process
WORKDIR /app

# Download ASIO and Crow libraries from GitHub
RUN git clone https://github.com/chriskohlhoff/asio.git asio
RUN git clone https://github.com/CrowCpp/Crow.git crow

# Copy your source code (main.cpp) into the container
COPY . .

# Build the application using g++
RUN g++ -Icrow/include -Iasio/asio/include -o todo_app main.cpp -lsqlite3 -lpthread

# Stage 2: Runtime Stage
# Use an official GCC image (same as the build stage) for runtime
FROM gcc:14.2.0

# Install only the runtime dependencies needed to run the app
RUN apt-get update && apt-get install -y \
    libsqlite3-0 \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory for the runtime container
WORKDIR /app

# Copy the compiled binary (todo_app) from the build stage into the runtime container
COPY --from=build /app/todo_app .

# Expose port 8080 for your app (modify this if needed based on your app's settings)
EXPOSE 8080

# Set the default command to run the application
CMD ["./todo_app"]
