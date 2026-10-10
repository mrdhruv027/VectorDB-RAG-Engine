# ==============================================================================
#  VectorDB Production Multi-Stage Dockerfile
#  Builds a minimal, secure container image for Linux cloud deployment
# ==============================================================================

# Stage 1: Build & Test
FROM gcc:14-bookworm AS builder

WORKDIR /app

# Copy source code and dependencies
COPY httplib.h main.cpp Makefile index.html ./
COPY src/ ./src/
COPY data/ ./data/

# Compile application and unit tests
RUN g++ -std=c++17 -O2 main.cpp -o db -lpthread
RUN g++ -std=c++17 -O2 src/tests.cpp -o run_tests -lpthread

# Run test suite to verify binary integrity before packaging
RUN ./run_tests

# Stage 2: Minimal Runtime
FROM debian:bookworm-slim AS runner

# Install essential runtime dependencies and curl for health checks
RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    ca-certificates \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled binary, web frontend, and initial data
COPY --from=builder /app/db /app/db
COPY --from=builder /app/index.html /app/index.html
COPY --from=builder /app/data /app/data

# Ensure data directory exists and has write permissions for persistence
RUN mkdir -p /app/data && chmod 777 /app/data

# Default port (Render, Railway, Fly.io dynamically override $PORT)
ENV PORT=8080
EXPOSE 8080

# Health check using the /health endpoint
HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
    CMD curl -f http://127.0.0.1:${PORT:-8080}/health || exit 1

# Start VectorDB HTTP server
CMD ["./db"]
