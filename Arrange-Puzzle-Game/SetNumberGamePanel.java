import javax.swing.*;
import java.awt.*;


class SetNumberGamePanel extends SetGamePanel {
    SetNumberGamePanel(JPanel gamePanel, int size, JTextField hitCountField, String file) {
        super(gamePanel, size, hitCountField, file);
        formatNumberGamePanel();
        for (int i = 0; i < size * size; i++) {
            gameButton[i].addActionListener(e -> {
                GameButton hit = (GameButton) e.getSource();
                if (hit.buttonActive(getMainLocation(), size)) {
                    int tempValue = gameButton[getMainLocation()].getValue();
                    gameButton[getMainLocation()].setValue(hit.getValue());
                    hit.setValue(tempValue);
                    formatNumberGamePanel();
                    if (confirmResult()) {
                        JOptionPane.showMessageDialog(null, "Win", "Win", JOptionPane.WARNING_MESSAGE);
                    }
                }
            });
        }
    }

    private void formatNumberGamePanel() {
        for (int i = 0; i < size * size; i++) {
            gameButton[i].setFont(new Font("黑体", Font.PLAIN, 32));
            gameButton[i].setBackground(Color.WHITE);
            gameButton[i].setForeground(Color.BLACK);
            gameButton[i].setText(String.valueOf(gameButton[i].getValue()));
        }
        gameButton[getMainLocation()].setBackground(Color.BLACK);
        gameButton[getMainLocation()].setText(null);
    }
}
