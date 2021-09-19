
class MouseInputWin {
public:
    void leftMouseDownAt(int x, int y);
    void rightMouseDownAt(int x, int y);
    void middleMouseDownAt(int x, int y);

    void leftMouseUpAt(int x, int y);
    void rightMouseUpAt(int x, int y);
    void middleMouseUpAt(int x, int y);

    void scrollDown(int lines);
    void scrollUp(int lines);
    void scrollLeft(int lines);
    void scrollRight(int lines);
};
