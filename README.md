# How to Integrate a New Repository

- [How to Integrate a New Repository](#how-to-integrate-a-new-repository)
  - [Introduction](#introduction)
  - [1. Create a new Repository](#1-create-a-new-repository)
    - [1.1 Add a branch protection rule](#11-add-a-branch-protection-rule)
      - [Why!?](#why)
    - [1.2 (Optional) Add repository secrets](#12-optional-add-repository-secrets)
  - [2. Clone the repository and setup the basic structure](#2-clone-the-repository-and-setup-the-basic-structure)
    - [2.1 .gitignore](#21-gitignore)
    - [2.2 .gitattributes](#22-gitattributes)
    - [2.3 Folders](#23-folders)
    - [2.4 Files](#24-files)
    - [2.5 README.md](#25-readmemd)
  - [3. (Optional) GitHub Workflows](#3-optional-github-workflows)
    - [3.1 Build Workflow](#31-build-workflow)
    - [3.2 Build \& Release Workflow](#32-build--release-workflow)
    - [3.3 GitOps Readme Update](#33-gitops-readme-update)
      - [Badges](#badges)
  - [4. Development Guidelines](#4-development-guidelines)
    - [4.1 Commits / Branches](#41-commits--branches)
    - [4.2 Conventions](#42-conventions)

## Introduction

This how-to/guide and associated guidelines and templates are intended to provide a structured approach to integrating a new repository into the Evert/Firmware organization. The guide is intended to be used as a reference for developers and team-leads to ensure that the repository is setup correctly and follows the conventions and guidelines set by the organization. 

The guide is divided into sections that cover the necessary steps to create a new repository, clone the repository and setup a basic structure, add GitHub workflows and development guidelines.

> [!NOTE]
> Throughout this guide think of the following principles; `consistency`, `readability`, `reliability` and `security`. The goal is to create a repository that is easy to work with, easy to understand, easy to maintain and secure.

## 1. Create a new Repository

Create a new repository at [New Repository](https://github.com/organizations/InSol-Tech/repositories/new) with the following naming convention: `firmware-*`. Fill in the necessary details, select `Private` and optionally a relevant `.gitignore` template. For more information, refer to:

[Repository Naming Conventions](https://evert-as.atlassian.net/wiki/spaces/Firmware/pages/173899777/Repository+Naming+Conventions)

[.gitignore](#gitignore)

### 1.1 Add a branch protection rule

Create a new branch rule at `https://github.com/InSol-Tech/{firmware-name}/settings/rules` with the following settings (keeping defaults):

- **Ruleset Name**: main
- **Enforcement Status**: Active
- **Targets**: Add Target -> "Include default branch" (main should be the default branch)
- **Restrict Deletions**: Enabled
- **Require Linear History**: Enabled
- **Require Pull Request Reviews**: Enabled
  - **Required approvals**: >= 1
  - **Allowed merge methods**: Squash only
- **Require status checks to pass**: Enabled (if applicable)
- **Block force push**: Enabled

> [!IMPORTANT]
> Repositories with build or test workflows should use this as a status check for pull-requests. The status check should be required for merging.

#### Why!?

- **Restrict Deletions**: Prevents accidental, or malicious, deletion of the main branch.
- **Require Linear History**: Ensures that the branch has a linear history (disallow merge commits from being pushed directly to that branch).
- **Require Pull Request Reviews**: Ensures that the code is reviewed by at least one other team member before being merged into the main branch.
- **Require status checks to pass**: Ensures that the code passes all checks before being merged into the main branch.
- **Block force push**: Prevents accidental, or malicious, force pushes to the main branch.

Basically by following these rules we get a clean, easy-to-follow commit log, making it simpler to review and dissect issues later. We enforce team-engagement and ensure that the code is reviewed by at least one other team member before being merged into the `main` branch as well as making sure that the `main` branch is always in a `stable` state.


### 1.2 (Optional) Add repository secrets

If the repository will contain workflow(s) that require access to other repositories or packages, `Personal Access Tokens` (PATs) should be created and added as secrets to the repository. The secrets can be added at `https://github.com/InSol-Tech/{firmware-name}/settings/secrets/actions`. This can later be used in the workflow files for authentication.

> [!CAUTION]
> Never expose sensitive information in the source-code or commit messages.

## 2. Clone the repository and setup the basic structure

### 2.1 .gitignore

:x: Never ignore source code, documentation and build-configurations that are platform agnostic.

:white_check_mark: Always ignore files produced by the build-process itself.

:white_check_mark: Always ignore local configuration files for IDE/tools.

:white_check_mark: Always ignore files containing sensitive information (secrets, api-keys, etc).

For more information, refer to:

[Ignoring Files](https://docs.github.com/en/get-started/getting-started-with-git/ignoring-files)

[https://github.com/github/gitignore](https://github.com/github/gitignore)

[gitignore.io](https://www.toptal.com/developers/gitignore)

### 2.2 .gitattributes

Add the following minimal `.gitattributes` file to the repository root to ensure that text files are normalized to LF line-endings:

```
# Auto detect text files and perform LF normalization
* text=auto
```

### 2.3 Folders

Example structure from a STM32/CubeMX project:

| Folder  | Description                                                    | Originator | gitignore | Remark                                                                   |
| ------- | -------------------------------------------------------------- | ---------- | --------- | ------------------------------------------------------------------------ |
| .vscode | Visual Studio Code settings (gitignored, setup is documented). | User       | Yes       |                                                                          |
| build   | CMake build directory.                                         | CMake      | Yes       |                                                                          |
| cmake   | CMake scripts.                                                 | CubeMX     | No        |                                                                          |
| Core    | Core files.                                                    | CubeMX     | No        | Naming violates our conventention, but CubeMX is generating this folder. |
| docs    | Documentation for the specific component/device.               | User       | No        |                                                                          |
| Drivers | STM32CubeMX generated drivers.                                 | CubeMX     | No        | Naming violates our conventention, but CubeMX is generating this folder. |
| libs    | External libraries (firmware-libraries as sub-module).         | User       | No        |                                                                          |
| src     | User source files.                                             | User       | No        |                                                                          |

### 2.4 Files

:white_check_mark: Always ignore local configuration files for IDE/tools.

Example files from a STM32/CubeMX project:

| File                  | Description               | Originator     | gitignore | Remark                                                          |
| --------------------- | ------------------------- | -------------- | --------- | --------------------------------------------------------------- |
| .gitattributes        | Git attributes file.      | User           | No        |                                                                 |
| .gitignore            | Git ignore file.          | User           | No        |                                                                 |
| .gitmodules           | Git sub-module file.      | User           | No        |                                                                 |
| .mxproject            | CubeMX project file.      | CubeMX         | No        |                                                                 |
| CMakeLists.txt        | CMake build script.       | CubeMX -> User | No        | CubeMx generates this file once, then it's up to the developer. |
| CMakePresets.json     | CMake presets file.       | User           | No        | Configure/Build presets for CMake.                              |
| startup_stm32xxxxxx.s | Startup file.             | CubeMX         | No        |                                                                 |
| STM32XXXXX_FLASH.ld   | Linker script.            | CubeMX         | No        |                                                                 |
| README.md             | Repository documentation. | User           | No        |                                                                 |

### 2.5 README.md

:white_check_mark: Always include a `README.md` file in the repository root. This readme should contain information about the `repository`, how to build, how to flash, how to test, etc. 

[README Template](how-to-integrate-a-new-repository/README-template.md)

> [!NOTE]
> Documentation related to the specific source-code/component should be placed in the `docs` folder as this folder is fetched and included during the `documentation-generation` process.

## 3. (Optional) GitHub Workflows

If the repository serves as a source for an executable/binary, it should contain a build-workflow. The workflow(s) should be triggered by a pull-request to the `main` branch and should be used for status checks. The workflow(s) should also be triggered by a tag and used for creating releases, basically following this pattern:

| Name                   | Description                                    | Initiator  |
| ---------------------- | ---------------------------------------------- | ---------- |
| build.yaml             | Build firmware, used for branch status checks. | PR -> main |
| build-and-release.yaml | Build and create releases.                     | Tag: v*    |

Workflow files must be placed in the `.github/workflows` folder.

It is also recommended to add status badges describing the latest build status and release status to the reposiories `README.md` file as well as the `firmware-gitops` repositories `README.md` file.

See [GitOps Readme Update](#33-gitops-readme-update) and [Badges](#badges) for more information.

### 3.1 Build Workflow

A `build.yaml` workflow serves the purpose of creating deterministic builds independent of the local environment used by the individual developers. The workflow should be triggered by a pull-request to the `main` branch and should be used for status checks. This ensures that the code pushed to main is at least buildable.

Example `build.yaml` workflow initiator:

```yaml
name: 'Build'
on: [pull_request, workflow_dispatch]

[...]
```

### 3.2 Build & Release Workflow

A `build-and-release.yaml` workflow serves the purpose of building and creating releases. The workflow should be triggered by a tag and should be used for creating releases. This workflow is basically `build.yaml` with additional steps for creating a release.

Example `build-and-release.yaml` workflow initiator:

```yaml
name: 'Build & Release'
on:
  push:
    tags:
      - 'v*'

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout repository
        uses: actions/checkout@v4
        with:
          token: ${{ secrets.PAT_FIRMWARE_LIBRARIES }}
          submodules: 'recursive'

[...]
```

> [!NOTE]
> Normally the internal `secrets.GITHUB_TOKEN` token is enough for authentication, but as the action needs access to a separate (probably private) repository the `PAT_FIRMWARE_LIBRARIES` secret is used for recursive checkouts of sub-modules. The PAT must be defined with access to all necessary repositories.

### 3.3 GitOps Readme Update

> [!IMPORTANT] 
> If any of the following sections should be updated under [GitOps Readme](README.md), please do so.

| Section                                                    | When?                                              |
| ---------------------------------------------------------- | -------------------------------------------------- |
| [Status](README.md#status)                                 | The repository contains status badges.             |
| [Personal Access Tokens](README.md#personal-access-tokens) | The repository defines the usage of a new PAT.     |
| [Packages](README.md#packages)                             | The repository defines the usage of a new package. |
| [Workflows](README.md#workflows)                           | The repository contains workflow files.            |
| [Workflows/Descriptions](README.md#descriptions)           | '                                                  |
| [Workflows/Usage](README.md#usage)                         | '                                                  |

#### Badges

Status-badges for workflow/actions can be generated like this:

```
Given {repo-name} = the name of the repository.
Given {workflow-name} = the name of the workflow (filename).

https://github.com/InSol-Tech/{repo-name}/actions/workflows/{workflow-name}.yaml/badge.svg

For a separate branch, add the branch name after the workflow-name:

Given {branch-name} = the name of the branch.
https://github.com/InSol-Tech/{repo-name}/actions/workflows/{workflow-name}.yaml/badge.svg?branch={branch-name}
```

For more information, refer to: [https://docs.github.com/en/actions/monitoring-and-troubleshooting-workflows/monitoring-workflows/adding-a-workflow-status-badge](https://docs.github.com/en/actions/monitoring-and-troubleshooting-workflows/monitoring-workflows/adding-a-workflow-status-badge)

## 4. Development Guidelines

The following guidelines are intended to provide a structured approach to development within the Evert/Firmware organization. The guidelines are intended to be used as a reference for developers and team-leads to ensure that the code is reviewed and follows conventions.

### 4.1 Commits / Branches

:x: Never keep sensitive information in the source-code or commit messages.

:x: Never commit code directly to the 'main' branch.

:white_check_mark: Always work in a separate feat/fix branch to avoid conflicts.

:white_check_mark: Always create pull-requests and request reviews from other-team members.

:white_check_mark: Always create version tags for releases.

> [!IMPORTANT]
> In general the 'main' branch should be guarded by a ruleset that requires status-checks, linear history and pull-requests with code-review before squash-merging. This is to ensure that the code is reviewed by at least one other team member before being merged into the main branch.

### 4.2 Conventions

- [Conventional Commits v1.0.0](https://www.conventionalcommits.org/en/v1.0.0/)








