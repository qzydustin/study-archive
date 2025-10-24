# Course-Assistant-Platform-Backend

## Introduction

This is the backend part of the Course Assistant Platform, complementing its frontend counterpart. The system is designed to offer a robust backend service supporting various academic tasks such as user, course, notification, assignment & exam, and score management. This project is built using Spring Boot and Maven, ensuring a solid, scalable, and easily maintainable backend architecture.

## Features

- **User Management:** Provides RESTful APIs to register student and teacher accounts and manage login functionalities.
- **Course Management:** Handles course content, schedules, and enrollment functionalities.
- **Notification Management:** Manages sending and storing notifications and messages.
- **Assignment & Exam Management:** Facilitates the distribution, collection, and grading of assignments.
- **Score Management:** Supports recording and analyzing student performance data.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

- Java Development Kit (JDK) installed on your local machine (version 17).
- Maven installed on your local machine.
- MySQL installed and running on your local machine.
- Familiarity with Spring Boot, Maven build lifecycle, and MySQL.

### Installation

1. Clone the repository to your local machine:
   ```sh
   git clone $URL$
   ```

2. Navigate to the project directory:
   ```sh
   cd Course-Assistant-Platform-Backend
   ```

3. Initialize the database using the `init.sql` file to create the necessary tables and structures:
   ```sh
   mysql -u [username] -p[password] < init.sql
   ```

4. Update the `src/main/resources/application.yml` file with the correct MySQL database connection configuration.

5. Use Maven to build the project:
   ```sh
   mvn clean install
   ```

6. Run the application:
   ```sh
   mvn spring-boot:run
   ```

The application should now be running on the configured port (default: 8080).
