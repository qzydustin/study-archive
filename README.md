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
   ```
   git clone https://github.com/UoA-BAT/PBMS.git
   cd PBMS
   ```

2. **Initialize the Database**
   ```
   python manage.py makemigrations
   python manage.py migrate
   ```

3. **Run the Development Server**
   ```
   python manage.py runserver
   ```
   Visit `http://127.0.0.1:8000/` in your web browser to view the application.

## License
This project is licensed under the [MIT License](LICENSE.md) - see the `LICENSE.md` file for details.
