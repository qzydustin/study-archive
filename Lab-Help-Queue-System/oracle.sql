DROP SEQUENCE ID;
CREATE SEQUENCE ID
    MINVALUE 1 NOMAXVALUE
    INCREMENT BY 1
    START
        WITH 1 NOCACHE;

DROP TABLE Events;
CREATE TABLE Events
(
    e_id            number,
    e_type          varchar(10),
    e_ws_name       varchar(15),
    e_role          varchar(10),
    e_class         varchar(10),
    e_class_section number(1),
    e_date          date,
    e_wait_time     INTERVAL DAY TO SECOND,
    PRIMARY KEY (e_id)
);
DROP TABLE Subject;
CREATE TABLE Subject
(
    s_subject   varchar(6),
    s_section   number(1),
    s_startTime date,
    s_endTime   date,
    s_startDate date,
    s_endDate   date,
    s_week      varchar(3) CHECK (s_week IN ('Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'))
);

CREATE TABLE adminAccount
(
    a_username varchar(10),
    a_password varchar(10),
    PRIMARY KEY (a_username)
);

INSERT INTO adminAccount
VALUES ('admin', 'admin');