#ifndef HOT_RELOAD_TEST_SRC_BASE_CURSOR_H_
#define HOT_RELOAD_TEST_SRC_BASE_CURSOR_H_

struct cursor {
  enum type {
    None = -1,
    Arrow,
    Text_Input,
    Resize_Vertical,
    Resize_Horizontal,
    Resize_All,
    Resize_Bottom_Left_Top_Right,
    Resize_Top_Left_Bottom_Right,
    Hand,
    Not_Allowed,

    Count
  };
};

#endif //HOT_RELOAD_TEST_SRC_BASE_CURSOR_H_
