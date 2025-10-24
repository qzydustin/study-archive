package administrator;

/**
 * @author qiz9744, wangs4830
 */
public class Administrator {

    /**
     * main
     *
     * @param args
     */
    public static void main(String[] args) {
        new Gui();
        if (args.length == 0) {
            new Communication("localhost");
        } else {
            new Communication(args[0]);
        }
    }
}
