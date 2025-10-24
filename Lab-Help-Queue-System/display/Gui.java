package display;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * @author qiz9744, wangs4830
 */
public class Gui extends JFrame {
    private static JLabel message;
    private static JLabel subject;
    private static JLabel section;
    private static JLabel[] labels;
    final int FRAME_WIDTH = 650;
    final int FRAME_HEIGHT = 650;

    /**
     * constructor
     */
    public Gui() {
        setSize(FRAME_WIDTH, FRAME_HEIGHT);
        setTitle("Help Queue - Display");
        addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                if (Communication.dis != null && Communication.dos != null) {
                    try {
                        Communication.dos.writeUTF("disconnect");
                    } catch (IOException ignored) {
                    }
                }
                System.exit(0);
            }
        });

        Font small = new Font("sans-serif", Font.BOLD, 25);
        Font medium = new Font("sans-serif", Font.BOLD, 30);
        Font large = new Font("sans-serif", Font.BOLD, 35);

        JPanel statePanel = new JPanel();
        JPanel informationPanel = new JPanel();
        JPanel tablePanel = new JPanel();
        JLabel title = new JLabel("Help Queue - Display", JLabel.CENTER);
        message = new JLabel(" ", JLabel.CENTER);
        JLabel room = new JLabel("Room: Phillips 115 Lab", JLabel.CENTER);
        subject = new JLabel("Class: UNKNOWN", JLabel.CENTER);
        section = new JLabel("Section: UNKNOWN", JLabel.CENTER);
        JLabel time = new JLabel("Time:", JLabel.CENTER);
        labels = new JLabel[10];
        for (int i = 0; i < labels.length; i++) {
            labels[i] = new JLabel();
            labels[i].setFont(medium);
        }
        JLabel tableName = new JLabel("Pos           " +
                "Name             " +
                "Time        " +
                "Wait Time", JLabel.CENTER);
        tableName.setFont(medium);
        title.setFont(large);
        message.setFont(medium);
        message.setForeground(Color.BLUE);
        room.setFont(small);
        subject.setFont(small);
        section.setFont(small);
        time.setFont(small);


        statePanel.setLayout(new GridLayout(2, 1));
        statePanel.add(title);
        statePanel.add(message);

        tablePanel.setLayout(new GridLayout(11, 1));

        informationPanel.setLayout(new GridLayout(2, 2));
        informationPanel.add(subject);
        informationPanel.add(section);
        informationPanel.add(room);
        informationPanel.add(time);

        Timer timer = new Timer(3000, new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                time.setText("Time: " + (new SimpleDateFormat("MM-dd-yy HH:mm:ss").format(new Date())));
                if (labels[0].getText().equals("")) {
                    message.setText("Message: no help queue");
                } else {
                    message.setText(" ");
                }
            }
        });
        timer.start();

        tablePanel.add(tableName);
        for (JLabel label : labels) {
            tablePanel.add(label);
        }

        add(statePanel, BorderLayout.NORTH);
        add(informationPanel, BorderLayout.SOUTH);
        add(tablePanel, BorderLayout.CENTER);

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
     * set/update help queue
     *
     * @param index   index
     * @param content content
     */
    public static void setLabels(int index, String content) {
        labels[index].setText(content);
    }

    /**
     * clear help queue
     */
    public static void clearLabels() {
        for (JLabel label : labels) {
            label.setText("");
        }
    }
}
