import javax.swing.*;

/**
 * GameButton
 *
 * @author qzydustin
 * @date 2018/8/25
 */

class GameButton extends JButton {
    private int value;
    private int index;

    GameButton(int index) {
        this.index = index;
    }

    boolean buttonActive(int mainLocation, int size) {
        boolean sameColumn = Math.abs(index - mainLocation) == size;
        boolean sameRow = mainLocation / size == index / size && Math.abs(index - mainLocation) == 1;
        return sameColumn || sameRow;
    }

    int getValue() {
        return value;
    }

    void setValue(int value) {
        this.value = value;
    }
}
