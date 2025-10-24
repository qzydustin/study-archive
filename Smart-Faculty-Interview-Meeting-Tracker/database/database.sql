CREATE DATABASE SFIMT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE SFIMT;
DROP TABLE IF EXISTS participant;
DROP TABLE IF EXISTS candidate;
DROP TABLE IF EXISTS position;
DROP TABLE IF EXISTS user_meeting;
DROP TABLE IF EXISTS message;
DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS meeting;
DROP TABLE IF EXISTS location;
# DROP TABLE IF EXISTS permission;

# CREATE TABLE permission
# (
#     u_type INT UNSIGNED PRIMARY KEY
# );

CREATE TABLE users
(
    u_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    email VARCHAR(100),
    u_password VARCHAR(20),
    phone_number VARCHAR(20),
    u_name VARCHAR(100),
    type INT /*0 for Super Admin,
                     1 for Admin,
                     2 for Department Admin,
                     3 for Meeting creator,
                     4 for Meeting participants from University,
                     5 for Candidate*/
#     FOREIGN KEY (u_type) REFERENCES permission (u_type)
);

CREATE TABLE location
(
    l_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    address VARCHAR(255),
		city VARCHAR(255)
);

CREATE TABLE participant
(
    u_id INT UNSIGNED PRIMARY KEY,
    available_start_date DATE,
		available_end_date DATE,
    available_start_hour TIME,
    available_end_hour   TIME,
    FOREIGN KEY (u_id) REFERENCES users (u_id)
);

CREATE TABLE position
(
    p_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    p_name VARCHAR(50),
    department VARCHAR(50)
);

CREATE TABLE candidate
(
    u_id INT UNSIGNED PRIMARY KEY,
    p_id INT UNSIGNED,
    cv VARCHAR(255),
    cover_letter VARCHAR(255),
		statement VARCHAR(255),
    FOREIGN KEY (u_id) REFERENCES users (u_id),
    FOREIGN KEY (p_id) REFERENCES position (p_id)
);

CREATE TABLE meeting
(
    m_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    start_time DATETIME,
    end_time DATETIME,
    l_id INT UNSIGNED,
		feedback VARCHAR(1000),
    status INT, /*0 for preparing,
                   1 for running,
                   2 for completed,
                   3 for cancelled*/
		
    FOREIGN KEY (l_id) REFERENCES location (l_id)
);

CREATE TABLE user_meeting
(
    u_id INT UNSIGNED,
    b_id INT UNSIGNED,
    PRIMARY KEY (u_id, b_id),
    FOREIGN KEY (u_id) REFERENCES users (u_id),
    FOREIGN KEY (b_id) REFERENCES meeting (m_id)
);

CREATE TABLE message
(
    b_id      INT UNSIGNED,
    m_id      INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    u_name    VARCHAR(100),
    m_body    VARCHAR(255),
    post_time DATETIME,
    FOREIGN KEY (b_id) REFERENCES meeting (m_id)
);