# This script is used to send score emails to students automatically.
# The script reads a csv file, which contains student ids and scores.
# The script sends an email to each student, with the score information.
# The script logs the sent emails in a log file.

# Example csv file:
# Name,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,B1,B2,B3,B4,B5,B6,B7,B8,B9,B10,B11,B12,B13,B14,B15,B16,B17,B18,B19,Total,Note
# AAA,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
# BBB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0


import csv
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

# Modify the following variables to fit your needs
csv_file = "score.csv"
sender = "sender@example.com"
smtp_server = "smtp.mailgun.org"
smtp_port = 587
username = "username"
password = "password"

# Modify the following code to fit your needs
with open(csv_file, "r") as file:
    reader = csv.reader(file)
    headers = next(reader)
    for row in reader:
        # row[0] is the student id
        # email is the student id + @ + example + .com
        # row[1:] are the scores
        id = row[0]
        email = id + "@example.com"
        score_lists = row[1:]

        msg = MIMEMultipart()
        msg["From"] = sender
        msg["To"] = email
        msg["Subject"] = "Demo Email"

        body = f"Dear {id},\n\n"
        body += "The score of demo is as following: \n"
        for header, score in zip(headers[1:], score_lists):
            body += f"{header}: {score}\n"
        body += "\nIf you have questions about grading, please contact me via email@example.com."
        body += "\nDo not reply to this email, I will not receive alerts, an automated script posts this email."
        body += "\n\nBest,\n xxx"
        msg.attach(MIMEText(body, "plain"))

        # Send the message via SMTP server.
        server = smtplib.SMTP(smtp_server, smtp_port)
        server.starttls()
        server.login(username, password)
        text = msg.as_string()
        server.sendmail(sender, email, text)
        server.quit()

        # Log the email
        with open("sent.log", "a") as log_file:
            log_file.write(text + "\n")
