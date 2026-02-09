# Backend Monitor Auth Setup (JWT Version)

## Database Setup

1. Same as before (assumes `users` table exists).

## Build and Run

1. Build the project:
   ```bash
   mkdir -p build
   cd build
   cmake ..
   make
   ```
2. Run the server:
   ```bash
   ./be_monitor
   ```

## API Endpoints

- `POST /api/register`: Same as before.
- `POST /api/login`: Returns `{"token": "ey...", ...}`.
- `POST /api/logout`: Stateless (client just discards token).
- `GET /api/me`: Requires Header `Authorization: Bearer <TOKEN>`.
