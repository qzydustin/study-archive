import javax.swing.*;
import java.awt.*;
import java.util.ArrayList;
import java.util.Random;

class SetGamePanel {
    GameButton[] gameButton;
    int size;
    private JTextField hitCountField;
    private JPanel gamePanel;
    String file;

    SetGamePanel(JPanel gamePanel, int size, JTextField hitCountField, String file) {
        //传参数
        this.size = size;
        this.hitCountField = hitCountField;
        this.file = file;
        this.gamePanel = gamePanel;
        gamePanel.setLayout(new GridLayout(size, size));
        do {
        //  setGameButton(); 随机打乱，已废弃
            setGameButtonSmart();
        } while (hard());
    }

    private void setGameButton() {
        gamePanel.removeAll();
        gameButton = new GameButton[size * size];
        int[] valueList = setValueList();
        for (int i = 0; i < size * size; i++) {
            gameButton[i] = new GameButton(i);
            gameButton[i].setValue(i);
            gameButton[i].setValue(valueList[i]);
            gamePanel.add(gameButton[i]);
        }
    }

    private void setGameButtonSmart() {
        gamePanel.removeAll();
        gameButton = new GameButton[size * size];
        for (int i = 0; i < size * size; i++) {
            gameButton[i] = new GameButton(i);
            gameButton[i].setValue(i + 1);
            gamePanel.add(gameButton[i]);
        }
        int times = size * size * size * size * size;
        for (int i = 0; i < times; i++) {
            int random = new Random().nextInt(size * size);
            if (gameButton[random].buttonActive(getMainLocation(), size)) {
                int tempValue = gameButton[getMainLocation()].getValue();
                gameButton[getMainLocation()].setValue(gameButton[random].getValue());
                gameButton[random].setValue(tempValue);
            }
        }
    }

    int getMainLocation() {
        for (int i = 0; i < size * size; i++) {
            if (gameButton[i].getValue() == size * size) {
                return i;
            }
        }
        return -1;
    }

    private int[] setValueList() {
        int n = size * size;
        int[] valueList = new int[n];
        ArrayList<Integer> temp = new ArrayList<>();
        ArrayList<Integer> list = new ArrayList<>();
        for (int i = 1; i <= n; i++) {
            temp.add(i);
        }
        int index = 0;
        while (list.size() != n) {
            int it = temp.get((int) (Math.random() * n));
            if (!list.contains(it)) {
                valueList[index] = it;
                list.add(it);
                index++;
            }
        }
        return valueList;
    }

    private boolean hard() {
        //难度参数
        int hardDegree = 0;
        int minHardDegree = size * size / 2;
        for (int i = 0; i < size * size; i++) {
            if (gameButton[i].getValue() != i + 1) {
                hardDegree++;
            }
        }
        if (hardDegree == 0) {
            JOptionPane.showMessageDialog(null,
                    "You are Lucky",
                    "Lucky", JOptionPane.WARNING_MESSAGE);
            return true;
        }
        return hardDegree <= minHardDegree;
    }

    boolean confirmResult() {
        int hitCount = Integer.parseInt(hitCountField.getText());
        hitCountField.setText(String.valueOf(--hitCount));
        int i;
        for (i = 0; i < size * size; i++) {
            if (gameButton[i].getValue() != i + 1) {
                break;
            }
        }
        if (i == size * size && hitCount >= 0) {
            return true;
        } else if (i != size * size && hitCount <= 0) {
            JOptionPane.showMessageDialog(null, "No remaining hit count", "Lose", JOptionPane.WARNING_MESSAGE);
        }
        return false;
    }
}
