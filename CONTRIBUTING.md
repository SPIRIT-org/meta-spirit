# Contributing to meta-spirit

Thank you for your interest in contributing! This document outlines how to set up your development environment and follow our code conventions.

## Development Environment

### Requirements

- Linux host (or WSL2 on Windows / Docker Desktop on macOS)
- Docker or Podman installed
- Python 3.9+ with `pip`
- [`pre-commit`](https://pre-commit.com/)

### Pre-commit Setup

This repository uses `pre-commit` to enforce code quality and commit conventions. Contributors are expected to run pre-commit before submitting changes.

#### Installation

Choose one of the following methods:

**Global install (user-wide):**
```sh
pip install --user pre-commit
```

**Python virtual environment:**
```sh
python -m venv .venv
source .venv/bin/activate
pip install pre-commit
```

**Nix shell:**
```sh
nix develop
```

#### Usage

After installing, install the hooks:
```sh
pre-commit install
```

Run on all files:
```sh
pre-commit run --all-files
```

## Code Conventions

### Commit Messages

This project uses [Conventional Commits](https://www.conventionalcommits.org/). Please format your commit messages as:

```
<type>(<scope>): <description>

[optional body]
```

Types:
- `feat`: new feature
- `fix`: bug fix
- `chore`: maintenance task
- `docs`: documentation
- `refactor`: code refactoring
- `dts`: device tree changes
- `recipe`: yocto recipe changes

Examples:
```
feat(dts): add Spirit Phone power configuration for CM5
fix: correct directories for linux-raspberrypi:do_configure
chore: update pre-commit configuration
```

### Pull Requests

- All commits must pass pre-commit checks
- PRs are automatically checked by GitHub Actions
- Use conventional commit messages in PR titles and descriptions
- Ensure `pre-commit run --all-files` passes before submitting

## Getting Help

- Open an issue at https://github.com/Synthxyl704/meta-spirit/issues
- Check existing issues and discussions before creating new ones