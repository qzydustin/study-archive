import javax.swing.*;
import java.awt.*;


class SetImageGamePanel extends SetGamePanel {
    private CutImage cutImage = new CutImage();

    SetImageGamePanel(JPanel gamePanel, int size, JTextField hitCountField, String file) {
        super(gamePanel, size, hitCountField, file);
        formatImageGamePanel();
        for (int i = 0; i < size * size; i++) {
            gameButton[i].addActionListener(e -> {
                GameButton hit = (GameButton) e.getSource();
                if (hit.buttonActive(getMainLocation(), size)) {
                    int tempValue = gameButton[getMainLocation()].getValue();
                    gameButton[getMainLocation()].setValue(hit.getValue());
                    hit.setValue(tempValue);
                    formatImageGamePanel();
                    if (confirmResult()) {
                        JOptionPane.showMessageDialog(null, "Win", "Win", JOptionPane.WARNING_MESSAGE);
                    }
                }
            });
        }
    }

    private void formatImageGamePanel() {
        Image image = Toolkit.getDefaultToolkit().getImage(file);
        Image[] images = cutImage.getImages(image, size);
        for (int i = 0; i < size * size; i++) {
            int temp = gameButton[i].getValue();
            gameButton[i].setIcon(new ImageIcon(images[temp - 1]));
            gameButton[i].setBackground(Color.WHITE);
            gameButton[i].setBorderPainted(false);
            gameButton[i].setBorder(null);
        }
        gameButton[getMainLocation()].setBackground(Color.BLACK);
        gameButton[getMainLocation()].setIcon(null);
    }
}
