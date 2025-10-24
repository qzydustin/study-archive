import javax.swing.*;

import java.awt.event.ActionEvent;

class MainGame extends JFrame {
    private int mainLocation;
    private JFrame frame;
    private JPanel mainPanel;
    private JPanel topPanel;
    private JPanel gamePanel;
    private JPanel imagePanel;
    private JTextField hitCountField;
    private JTextField sizeField;
    private JLabel sizeLabel;
    private JLabel hitCountLabel;
    private JLabel imageLabel;
    private JButton image;
    private JButton number;
    private ImageIcon imageIcon;
    private String file;

    private MainGame() {
        frame = new JFrame("MainGame");
        frame.setContentPane(mainPanel);

        //添加菜单栏
        JMenuBar bar = new JMenuBar();
        JMenu menu = new JMenu("说明");
        JMenuItem about = new JMenuItem("关于");
        bar.add(menu);
        menu.add(about);
        about.addActionListener((ActionEvent e)
                -> JOptionPane.showMessageDialog
                (null,
                        "作者：qzydustin 版本：Alpha",
                        "About", JOptionPane.PLAIN_MESSAGE));
        frame.setJMenuBar(bar);
        //添加退出按钮
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        //自动调整frame大小
        frame.pack();
        frame.setVisible(true);

        //number按钮监听器
        number.addActionListener((ActionEvent e) -> start(0));
        //image按钮监听器
        image.addActionListener(e -> {
            JFileChooser fileChooser = new JFileChooser("Picture");
            fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
            fileChooser.showDialog(new JLabel(), "选择");
            file = fileChooser.getSelectedFile().getAbsolutePath();
            imageIcon = new ImageIcon(file);
            start(1);
        });
    }

    private void start(int mode) {
        //获取游戏长宽大小
        int size = Integer.parseInt(sizeField.getText());
        //清空imageLabel
        imageLabel.setIcon(null);
        //开始初始化游戏，mode是0生成数字游戏，1生成图像游戏
        if (mode == 0) {
            new SetNumberGamePanel(gamePanel, size, hitCountField, file);
        } else if (mode == 1) {
            imageLabel.setIcon(imageIcon);
            new SetImageGamePanel(gamePanel, size, hitCountField, file);
        }
        //自动调整frame大小
        frame.pack();
    }

    public static void main(String[] args) {
        new MainGame();
    }

}
