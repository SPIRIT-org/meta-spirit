# Contributing to meta-spirit

## Development Environment

### Requirements

- Linux host (or WSL2 on Windows / Docker Desktop on macOS)
- Docker or Podman installed
- Python 3.9+ with `pip`
- [`pre-commit`](https://pre-commit.com/)

### Pre-commit Setup

This repository uses `pre-commit` to enforce code quality and commit conventions. 
As a result, contributors are encouraged to run pre-commit before submitting changes!

#### Installation

You can either choose one of the following methods or as per what is needed:

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

#### Usage

After installing, install the hooks (in the forked directory):
```sh
pre-commit install
```

Run on all files:
```sh
pre-commit run --all-files # make sure it all passes
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

- You should and must ensure that ALL your commits pass pre-commit checks
- PRs are automatically checked by GitHub Actions
- Use conventional commit messages in PR titles and descriptions 
- Again, ensure `pre-commit run --all-files` passes before submitting 
