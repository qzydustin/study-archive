# PBMS

## Overview

The Product Backlog Management System (PBMS) is an Agile project management tool designed to facilitate the management of Product Backlogs. It allows users to effectively organize, prioritize, and update their product backlog items, ensuring a smooth and efficient product development process.

## Features

- **Backlog Item Creation**: Create and define new backlog items with descriptions, acceptance criteria, and more.
- **Prioritization**: Prioritize items in your backlog to ensure that your team is working on the most important tasks.

## Getting Started

### Prerequisites

Before you begin, ensure you have met the following requirements:

- Python 3.11.7
- Django 5.0.1

### Setup and Installation

1. **Clone the Repository**

   ```shell
   git clone https://github.com/UoA-BAT/PBMS.git
   cd PBMS
   ```

2. **Initialize the Database**

   ```shell
   python manage.py makemigrations
   python manage.py migrate
   ```

3. **Run the Development Server**

   ```shell
   python manage.py runserver
   ```

   Visit `http://127.0.0.1:8000/` in your web browser to view the application.

## Ensuring Code Quality with Pre-commit

To maintain the quality and consistency of our code, we've integrated [pre-commit](https://pre-commit.com/) into our project. Pre-commit is a tool that uses various hooks to automatically check for code issues, such as formatting discrepancies and syntax errors, before code commits.

### Setting Up

Before you start contributing, please ensure you have pre-commit installed. If you haven't installed it yet, you can do so with the following command:

```sh
pip install pre-commit
```

After installation, run the following command in the root directory of the project to install the git hooks:

```sh
pre-commit install
```

This will ensure that the configured hooks are automatically run before each commit.

### How to Use

Once the git hooks are installed, no further action is required on your part. Each time you execute the `git commit` command, the pre-commit hooks will automatically run and check if your code complies with our standards.

If the checks fail, pre-commit will prevent the commit and provide error messages indicating what went wrong and how to fix it. Please address these issues according to the prompts and try to commit again.

You can also manually run all configured hooks against all files, not just the ones you're attempting to commit:

```sh
pre-commit run --all-files
```

### Updating Pre-commit Configuration

The pre-commit configuration of the project may get updated from time to time. To ensure you're using the latest hook configurations, regularly run the following command to update your local hooks:

```sh
pre-commit autoupdate
```

This will check for and update your hooks to the latest versions.

We encourage all contributors to follow these guidelines to help us keep the codebase clean and consistent. If you have any questions about pre-commit, feel free to ask in the project's issue tracker.

## License

This project is licensed under the [MIT License](LICENSE.md) - see the `LICENSE.md` file for details.
