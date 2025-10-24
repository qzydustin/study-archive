import javax.swing.*;
import java.awt.*;
import java.awt.image.*;

class CutImage extends JComponent {

    Image[] getImages(Image image, int size) {
        Image[] blockImage = new Image[size * size];
        Toolkit tool = getToolkit();
        int imageWidth = image.getWidth(this);
        int imageHeight = image.getHeight(this);
        int w = imageWidth / size;
        int h = imageHeight / size;
        int k = 0;
        PixelGrabber pixelGrabber;
        ImageProducer imageProducer;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                int[] pixels = new int[w * h];
                pixelGrabber = new PixelGrabber(image, j * w, i * h, w, h, pixels, 0, w);
                try {
                    pixelGrabber.grabPixels();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                imageProducer = new MemoryImageSource(w, h, pixels, 0, w);
                blockImage[k] = tool.createImage(imageProducer);
                k++;
            }
        }
        return blockImage;
    }
}
