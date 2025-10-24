package administrator;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

/**
 * @author qiz9744, wangs4830
 */
public class Gui extends JFrame {
    private static JLabel message;
    private static JLabel subject;
    private static JLabel section;
    final int FRAME_WIDTH = 700;
    final int FRAME_HEIGHT = 700;

    /**
     * constructor
     */
    public Gui() {
        setSize(FRAME_WIDTH, FRAME_HEIGHT);
        setTitle("Help Queue - Administrator");

        JLabel title = new JLabel("Help Queue - Administrator", JLabel.CENTER);
        title.setFont(new Font("sans-serif", Font.BOLD, 30));
        message = new JLabel("", JLabel.CENTER);
        message.setForeground(Color.BLUE);
        message.setFont(new Font("sans-serif", Font.BOLD, 25));

        JPanel botPanel = new JPanel();
        botPanel.setLayout(new GridLayout(18, 3));
        JTextField subjectTextField = new JTextField();
        JTextField sectionTextField = new JTextField();
        JTextField username = new JTextField();
        JTextField password = new JTextField();
        JButton login = new JButton("Login");
        JTextField week = new JTextField();
        JTextField startTime = new JTextField();
        JTextField endTime = new JTextField();
        JTextField startDate = new JTextField();
        JTextField endDate = new JTextField();
        JButton subjectButton = new JButton("Submit");
        subject = new JLabel("Project: UNKNOWN", JLabel.CENTER);
        section = new JLabel("Section: UNKNOWN", JLabel.CENTER);
        JTextField cancelHelp = new JTextField();
        JButton cancelHelpButton = new JButton("Cancel Help");
        JButton initializeButton = new JButton("Initialize help queue");
        JButton initializeSubjectButton = new JButton("Initialize subject database");
        JButton initializeEventButton = new JButton("Initialize event database");

        botPanel.add(subject);
        botPanel.add(section);
        botPanel.add(new JLabel("Room: Phillips 115 Lab", JLabel.CENTER));

        botPanel.add(new JLabel());
        botPanel.add(new JLabel());
        botPanel.add(new JLabel());

        botPanel.add(new JLabel("Username: ", JLabel.CENTER));
        botPanel.add(username);
        botPanel.add(new JLabel());

        botPanel.add(new JLabel("Password: ", JLabel.CENTER));
        botPanel.add(password);
        botPanel.add(login);

        botPanel.add(new JLabel());
        botPanel.add(new JLabel());
        botPanel.add(new JLabel());

        botPanel.add(new JLabel());
        botPanel.add(new JLabel("Add Subject", JLabel.CENTER));
        botPanel.add(new JLabel());

        botPanel.add(new JLabel("Subject: ", JLabel.CENTER));
        botPanel.add(subjectTextField);
        botPanel.add(new JLabel("Format: CS 260", JLabel.CENTER));

        botPanel.add(new JLabel("Section: ", JLabel.CENTER));
        botPanel.add(sectionTextField);
        botPanel.add(new JLabel("Format: 1", JLabel.CENTER));

        botPanel.add(new JLabel("Week: ", JLabel.CENTER));
        botPanel.add(week);
        botPanel.add(new JLabel("Format: Fri", JLabel.CENTER));

        botPanel.add(new JLabel("Start Time: ", JLabel.CENTER));
        botPanel.add(startTime);
        botPanel.add(new JLabel("Format: 13:00:00", JLabel.CENTER));

        botPanel.add(new JLabel("End Time: ", JLabel.CENTER));
        botPanel.add(endTime);
        botPanel.add(new JLabel("Format: 15:00:00", JLabel.CENTER));

        botPanel.add(new JLabel("Start Date: ", JLabel.CENTER));
        botPanel.add(startDate);
        botPanel.add(new JLabel("Format: 2020-01-01", JLabel.CENTER));

        botPanel.add(new JLabel("End Time: ", JLabel.CENTER));
        botPanel.add(endDate);
        botPanel.add(new JLabel("Format: 2020-12-31", JLabel.CENTER));

        botPanel.add(new JLabel());
        botPanel.add(subjectButton);
        botPanel.add(new JLabel());

        botPanel.add(new JLabel());
        botPanel.add(new JLabel());
        botPanel.add(new JLabel());

        botPanel.add(new JLabel("Client Name: ", JLabel.CENTER));
        botPanel.add(cancelHelp);
        botPanel.add(cancelHelpButton);

        botPanel.add(new JLabel());
        botPanel.add(new JLabel());
        botPanel.add(new JLabel());

        botPanel.add(initializeButton);
        botPanel.add(initializeSubjectButton);
        botPanel.add(initializeEventButton);


        add(title, BorderLayout.NORTH);
        add(message, BorderLayout.CENTER);
        add(botPanel, BorderLayout.SOUTH);
        setVisible(true);


        addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                if (Communication.dis != null && Communication.dos != null) {
                    Communication.send("disconnect");
                }
                System.exit(0);
            }
        });

        subjectButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String str = "add: " + subjectTextField.getText() + "#" +
                        sectionTextField.getText() + "#" +
                        startTime.getText() + "," + endTime.getText() + "#" +
                        week.getText() + "#" +
                        startDate.getText() + "," + endDate.getText();
                Communication.send(str);
            }
        });

        login.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Communication.send("login: " + username.getText() + "\n" + password.getText());
            }
        });

        initializeButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Communication.send("initialize queue");
            }
        });

        initializeSubjectButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Communication.send("initialize subject");
            }
        });

        initializeEventButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Communication.send("initialize event");
            }
        });

        cancelHelpButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Communication.send("cancel: " + cancelHelp.getText());
            }
        });
    }

    /**
     * set message
     *
     * @param msg message
     */
    public static void setMessage(String msg) {
        message.setText(msg);
    }

    /**
     * set subject
     *
     * @param sub subject
     */
    public static void setSubject(String sub) {
        subject.setText(sub);
    }

    /**
     * set section
     *
     * @param sec section
     */
    public static void setSection(String sec) {
        section.setText(sec);
    }

}
