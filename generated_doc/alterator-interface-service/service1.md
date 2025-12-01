[English](service1.md) | [Русский](service1.ru_RU.md)

## Overview

The `org.altlinux.alterator.service1` interface defines lifecycle operations for Alterator-managed services. Introspection data is installed to `/usr/share/dbus-1/interfaces/org.altlinux.alterator.service1.xml`; PolicyKit defaults are provided by `/usr/share/polkit-1/actions/org.altlinux.alterator.service1.policy`. Methods return the response code of the executed backend (`0` — success, `!= 0` — failure). Parameters that carry configuration are passed as a single string to `stdin` of the backend process.

## `Info() -> (stdout_bytes: ay, response: i)`

- Purpose: provide Alterator Entry description of the service tool (`service` entity).
- Parameters: none; returns `stdout_bytes` with the entry and `response`.
- Expected behavior (example): `stdout_bytes` contains the serialized entry, `response = 0`.

## `Deploy(stdin: s) -> (response: i)`

- Purpose: deploy the service using parameters supplied via `stdin`.
- Parameters: `stdin` — configuration string; returns `response`.
- Expected behavior (example): backend applies the configuration, `response = 0` on success.

## `Start() -> (response: i)`

- Purpose: start systemd units used by the service.
- Parameters: none; returns `response`.
- Expected behavior (example): units are started, `response = 0`; failure to start yields `response != 0`.

## `Stop() -> (response: i)`

- Purpose: stop systemd units used by the service.
- Parameters: none; returns `response`.
- Expected behavior (example): units are stopped, `response = 0`; errors produce a non-zero code.

## `Configure(stdin: s) -> (response: i)`

- Purpose: adjust settings of an already deployed service.
- Parameters: `stdin` — configuration string; returns `response`.
- Expected behavior (example): configuration is applied, `response = 0`.

## `Undeploy(stdin: s) -> (response: i)`

- Purpose: disable or remove the deployed service.
- Parameters: `stdin` — data that defines the teardown procedure; returns `response`.
- Expected behavior (example): service is undeployed according to input, `response = 0`.

## `Backup(stdin: s) -> (response: i)`

- Purpose: create a backup for the service using parameters from `stdin`.
- Parameters: `stdin` — backup specification; returns `response`.
- Expected behavior (example): backup files are produced, `response = 0`.

## `Restore(stdin: s) -> (response: i)`

- Purpose: restore the service from a backup according to parameters supplied via `stdin`.
- Parameters: `stdin` — restore specification; returns `response`.
- Expected behavior (example): service state is restored, `response = 0`; failure returns `response != 0`.

## `Status() -> (stdout_bytes: ay, response: i)`

- Purpose: report current service state.
- Parameters: none; returns `stdout_bytes` with status data and `response`.
- Expected behavior (example): `stdout_bytes` describes whether the service is deployed and how it was deployed, `response = 0`.

## Signals

### `service_stderr_signal(line: s)`

- Purpose: deliver stderr lines from the backend process asynchronously.
- Payload: one line from the `stderr` stream per signal emission.

### `service_stdout_signal(line: s)`

- Purpose: deliver stdout lines from the backend process asynchronously.
- Payload: one line from the `stdout` stream per signal emission.
