package client;

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
    private static JButton helpButton;
    private static JButton cancelButton;
    final int FRAME_WIDTH = 900;
    final int FRAME_HEIGHT = 300;

    /**
     * constructor
     */
    public Gui() {
        setSize(FRAME_WIDTH, FRAME_HEIGHT);
        setTitle("Help Queue - Client");
        addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                if (Communication.dis != null && Communication.dos != null) {
                    Communication.send("disconnect");
                }
                System.exit(0);
            }
        });

        Font small = new Font("sans-serif", Font.BOLD, 25);
        Font medium = new Font("sans-serif", Font.BOLD, 30);
        Font large = new Font("sans-serif", Font.BOLD, 35);
        Font extraLarge = new Font("sans-serif", Font.BOLD, 50);

        JPanel statePanel = new JPanel();
        JPanel informationPanel = new JPanel();
        JPanel buttonPanel = new JPanel();
        JLabel title = new JLabel("Help Queue - Client", JLabel.CENTER);
        message = new JLabel("", JLabel.CENTER);
        JLabel room = new JLabel("Room: Phillips 115 Lab", JLabel.CENTER);
        subject = new JLabel("Project: UNKNOWN", JLabel.CENTER);
        section = new JLabel("Section: UNKNOWN", JLabel.CENTER);
        helpButton = new JButton("Help");
        cancelButton = new JButton("Cancel");

        title.setFont(large);
        message.setFont(medium);
        message.setForeground(Color.BLUE);
        helpButton.setFont(extraLarge);
        cancelButton.setFont(extraLarge);
        helpButton.setForeground(Color.GREEN);
        cancelButton.setForeground(Color.RED);
        room.setFont(small);
        subject.setFont(small);
        section.setFont(small);
        helpButton.setEnabled(false);
        cancelButton.setEnabled(false);

        helpButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Communication.send("request");
            }
        });
        cancelButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Communication.send("cancel");
            }
        });

        statePanel.setLayout(new GridLayout(2, 1));
        statePanel.add(title);
        statePanel.add(message);

        informationPanel.setLayout(new GridLayout(2, 2));
        informationPanel.add(subject);
        informationPanel.add(section);
        informationPanel.add(room);

        buttonPanel.setLayout(new GridLayout(1, 2));
        buttonPanel.add(helpButton);
        buttonPanel.add(cancelButton);

        add(statePanel, BorderLayout.NORTH);
        add(buttonPanel, BorderLayout.CENTER);
        add(informationPanel, BorderLayout.SOUTH);

        setVisible(true);
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

    /**
     * set button enabled
     */
    public static void setButtonEnabled() {
        helpButton.setEnabled(true);
        cancelButton.setEnabled(true);
    }

    /**
     * set button disabled
     */
    public static void setButtonDisabled() {
        helpButton.setEnabled(false);
        cancelButton.setEnabled(false);
    }
}