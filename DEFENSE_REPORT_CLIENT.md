# Currency System Client: Report for Defense

## 1. Qt capabilities and UI stack used in the client

The client is built on Qt 6 Widgets and uses a classic desktop architecture with page-based navigation.

### Core Qt modules

- `QtWidgets`: main UI framework (`QMainWindow`, pages, forms, tables, buttons, labels, dialogs).
- `QtNetwork`: TCP communication (`QTcpSocket`) with the server via a text protocol.
- `QtCharts`: rendering and exporting history chart widget.
- `QtXml`: used in protocol/parsing and dependencies.

### Main widgets and layout approach

- Window shell: `QMainWindow` + left menu + central `QStackedWidget`.
- Navigation and content:
  - `BurgerMenuWidget` (left panel),
  - pages: `StartPage`, `DashboardPage`, `SettingsPage`.
- Layout system:
  - `QHBoxLayout`, `QVBoxLayout`, `QGridLayout`, `QFormLayout`.
- Tables/charts:
  - `QTableView` through Qt models,
  - chart widget for history visualization.
- Controls:
  - `QComboBox`, `QDateEdit`, `QDoubleSpinBox`, `QLineEdit`, `QPushButton`, `QLabel`.
- Styling:
  - global QSS (`client/assets/styles/app.qss`),
  - object-name scoped styles for pages/components.

## 2. Threading model and responsiveness

The client is split into two execution contexts:

- **UI thread**:
  - owns all widgets and renders UI,
  - handles user interaction and state updates.
- **Network thread** (`QThread`):
  - started in `ClientApplication`,
  - `ServerGateway` is moved to this thread (`moveToThread`),
  - network calls are dispatched via `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`.

This separates rendering from blocking socket operations and prevents UI freezing during requests.

At shutdown, the network thread is stopped safely:

- explicit disconnect call via `BlockingQueuedConnection`,
- `quit()` + `wait()`.

## 3. Client architecture

### High-level layers

- **Views** (`client/src/views`):
  - page widgets and reusable visual components.
- **Controllers** (`client/src/controllers`):
  - orchestration of UI events, state mutation, and service calls.
- **Models/State** (`client/src/models`):
  - `AppState` (application state + signals),
  - table models for rates/aggregation.
- **Services** (`client/src/services`):
  - business orchestration (`ApiOrchestratorService`, `AggregationService`, export/chart prep).
- **Gateway** (`client/src/gateways/ServerGateway`):
  - transport-level server communication.
- **Serialization/DTO**:
  - text protocol encoding/decoding and request/response DTOs.

### Role of `AppState`

`AppState` is the state hub for:

- selected sources and currency catalogs,
- latest rates, history points, aggregation summary,
- connection status and server settings,
- conversion result.

It emits Qt signals (`...Changed`, `errorRaised`, `infoRaised`) to keep controllers and UI synchronized.

## 4. How a request is executed and processed

Example flow (load rates/history):

1. User action on `DashboardPage` button.
2. `DashboardController` validates input and starts busy state.
3. Controller schedules network operation into the network thread.
4. `ApiOrchestratorService` builds DTO request.
5. `ServerGateway` encodes command via `TextProtocolCodec`, sends TCP request, waits for response line.
6. Response is decoded to envelope and mapped to domain/client DTOs.
7. Result is marshaled back to UI thread.
8. Controller updates `AppState`, table models, summary widgets, and user notifications.

Error handling:

- transport errors vs provider/domain errors are classified separately,
- user gets explicit message category,
- connection status in UI is updated accordingly.

## 5. Client tests (GoogleTest format) and coverage

Client tests are now in GoogleTest style and executed via `gtest_discover_tests`.

### Test entrypoint

- `client/tests/ClientTests.cpp`:
  - creates `QApplication`,
  - runs `RUN_ALL_TESTS()`.

### Test groups and what they validate

- `ServerGatewayTests`:
  - endpoint validation,
  - refused connection handling,
  - request/response round-trip over mock TCP server,
  - provider scoping in requests,
  - malformed protocol response handling.

- `ApiOrchestratorServiceTests`:
  - currency catalog fetching and sorting,
  - provider forwarding to server,
  - protocol validation for malformed payload,
  - history request validation,
  - conversion response parsing and malformed response rejection.

- `ExportServiceTests`:
  - export to JSON/CSV/XLSX,
  - validation for unsupported format,
  - empty data rejection for rates/history,
  - CSV escaping for commas and quotes.

- `HistoryChartWidgetTests`:
  - chart PNG export with points,
  - placeholder PNG export for empty dataset.

### Anti-hang safeguards in tests

- GoogleTest discovery timeout (`DISCOVERY_TIMEOUT 60`),
- per-test timeout in CTest (`TIMEOUT 90`),
- CI `ctest --timeout 120`,
- polling helpers with explicit timeout instead of indefinite waits.

## 6. CI execution and remote-branch behavior

CI workflow is configured to run on remote branches and PRs:

- push branches: `**`,
- pull requests: enabled.

Two jobs:

- Linux job for server build/tests,
- Windows job for client build/tests (Qt + MinGW setup).

Both jobs include explicit time limits to prevent endless execution.

## 7. Push readiness and repository hygiene

To prepare for clean remote pushes:

- root `.gitignore` was updated for technical/runtime files:
  - build outputs and binaries,
  - IDE temp folders,
  - Python cache/temp runtime folders,
  - runtime logs/dumps.
- temporary local extraction folder `tools/runtime_tmp` was removed.
- logging infrastructure sources under `logs/` are no longer globally hidden by ignore rules (only runtime log files are ignored).

---

If needed for defense slides, this report can be split into:
1) architecture diagram, 2) request-sequence diagram, 3) testing matrix and CI guarantees.
