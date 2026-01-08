#pragma once

#include <array>
#include <cstdint>
#include <vector>

class SimGu7000Real {
 public:
  // Display dimensions
  static constexpr uint16_t DISPLAY_WIDTH = 112;
  static constexpr uint16_t DISPLAY_HEIGHT = 16;

  // Font dimensions (5x7)
  static constexpr uint8_t FONT_WIDTH = 5;
  static constexpr uint8_t FONT_HEIGHT = 7;

  // Command constants
  static constexpr uint8_t CMD_CHARACTER_DISPLAY_START = 0x20;
  static constexpr uint8_t CMD_CHARACTER_DISPLAY_END = 0xFF;
  static constexpr uint8_t CMD_BACKSPACE = 0x08;
  static constexpr uint8_t CMD_HORIZONTAL_TAB = 0x09;
  static constexpr uint8_t CMD_LINE_FEED = 0x0A;
  static constexpr uint8_t CMD_HOME_POSITION = 0x0B;
  static constexpr uint8_t CMD_CARRIAGE_RETURN = 0x0D;
  static constexpr uint8_t CMD_DISPLAY_CLEAR = 0x0C;
  static constexpr uint8_t CMD_INITIALIZE_DISPLAY = 0x1B;
  static constexpr uint8_t CMD_SPECIFY_DOWNLOAD_REGISTER = 0x1B;
  static constexpr uint8_t CMD_DOWNLOAD_CHARACTER = 0x1B;
  static constexpr uint8_t CMD_DELETE_DOWNLOADED_CHARACTER = 0x1B;
  static constexpr uint8_t CMD_SPECIFY_INTERNATIONAL_FONT_SET = 0x1B;
  static constexpr uint8_t CMD_SPECIFY_CHARACTER_CODE_TYPE = 0x1B;
  static constexpr uint8_t CMD_OVERWRITE_MODE = 0x1F;
  static constexpr uint8_t CMD_VERTICAL_SCROLL_MODE = 0x1F;
  static constexpr uint8_t CMD_HORIZONTAL_SCROLL_MODE = 0x1F;
  static constexpr uint8_t CMD_CURSOR_SET = 0x1F;
  static constexpr uint8_t CMD_WAIT = 0x1F;
  static constexpr uint8_t CMD_SCROLL_DISPLAY_ACTION = 0x1F;
  static constexpr uint8_t CMD_DISPLAY_BLINK = 0x1F;
  static constexpr uint8_t CMD_SCREEN_SAVER = 0x1F;
  static constexpr uint8_t CMD_REAL_TIME_BIT_IMAGE_DISPLAY = 0x1F;
  static constexpr uint8_t CMD_CHARACTER_FONT_WIDTH_AND_SPACE = 0x1F;
  static constexpr uint8_t CMD_FONT_MAGNIFICATION_SET = 0x1F;
  static constexpr uint8_t CMD_CURRENT_WINDOW_SELECT = 0x1F;
  static constexpr uint8_t CMD_USER_WINDOW_DEFINITION_CANCEL = 0x1F;
  static constexpr uint8_t CMD_WRITE_SCREEN_MODE_SELECT = 0x1F;
  static constexpr uint8_t CMD_BRIGHTNESS_CONTROL = 0x1F;
  static constexpr uint8_t CMD_SPECIFY_OR_CANCEL_REVERSE_DISPLAY = 0x1F;
  static constexpr uint8_t CMD_HORIZONTAL_SCROLL_SPEED = 0x1F;
  static constexpr uint8_t CMD_SPECIFY_WRITE_MIXTURE_DISPLAY_MODE = 0x1F;

  SimGu7000Real();

  // Command processing
  void ProcessCommand(const std::vector<uint8_t>& command);

  // Display memory access
  const std::array<std::array<bool, DISPLAY_HEIGHT>, DISPLAY_WIDTH>&
  GetDisplayMemory() const;
  void ClearDisplayMemory();

  // Cursor management
  void SetCursor(uint16_t x, uint16_t y);
  uint16_t GetCursorX() const;
  uint16_t GetCursorY() const;

  // Character rendering
  void DrawCharacter(uint8_t character);
  void DrawCharacterAt(uint16_t x, uint16_t y, uint8_t character);

 private:
  // Display memory: 112x16 pixels as bool array
  // Each element represents one pixel
  std::array<std::array<bool, DISPLAY_HEIGHT>, DISPLAY_WIDTH> display_memory_;

  // Cursor position (in pixels)
  uint16_t cursor_x_;
  uint16_t cursor_y_;

  // Font and display settings
  uint8_t international_font_set_;
  uint8_t character_code_type_;
  bool overwrite_mode_;
  uint8_t scroll_mode_;
  uint8_t horizontal_scroll_speed_;
  uint8_t brightness_level_;
  bool reverse_display_;
  uint8_t composition_mode_;
  uint8_t current_window_;
  uint8_t font_magnification_x_;
  uint8_t font_magnification_y_;

  // Command state tracking
  bool waiting_for_command_;
  std::vector<uint8_t> command_buffer_;

  // Helper methods
  void SetPixel(uint16_t x, uint16_t y, bool on);
  bool GetPixel(uint16_t x, uint16_t y) const;
  void DrawFontCharacter(uint16_t x, uint16_t y, uint8_t character,
                         uint8_t magnification_x, uint8_t magnification_y);
  const uint8_t* GetFontData(uint8_t character) const;

  // Command implementations
  void ProcessCharacterDisplay(uint8_t character);
  void ProcessBackspace();
  void ProcessHorizontalTab();
  void ProcessLineFeed();
  void ProcessHomePosition();
  void ProcessCarriageReturn();
  void ProcessDisplayClear();
  void ProcessInitializeDisplay(const std::vector<uint8_t>& params);
  void ProcessCursorSet(const std::vector<uint8_t>& params);
  void ProcessDisplayClearCommand();
  void ProcessBrightnessControl(const std::vector<uint8_t>& params);
  void ProcessReverseDisplay(const std::vector<uint8_t>& params);
  void ProcessHorizontalScrollSpeed(const std::vector<uint8_t>& params);
  void ProcessCompositionMode(const std::vector<uint8_t>& params);
  void ProcessInternationalFontSet(const std::vector<uint8_t>& params);
  void ProcessCharacterCodeType(const std::vector<uint8_t>& params);
  void ProcessOverwriteMode(const std::vector<uint8_t>& params);
  void ProcessVerticalScrollMode(const std::vector<uint8_t>& params);
  void ProcessHorizontalScrollMode(const std::vector<uint8_t>& params);
  void ProcessWait(const std::vector<uint8_t>& params);
  void ProcessScrollDisplayAction(const std::vector<uint8_t>& params);
  void ProcessDisplayBlink(const std::vector<uint8_t>& params);
  void ProcessScreenSaver(const std::vector<uint8_t>& params);
  void ProcessRealTimeBitImageDisplay(const std::vector<uint8_t>& params);
  void ProcessCharacterFontWidthAndSpace(const std::vector<uint8_t>& params);
  void ProcessFontMagnificationSet(const std::vector<uint8_t>& params);
  void ProcessCurrentWindowSelect(const std::vector<uint8_t>& params);
  void ProcessUserWindowDefinitionCancel(const std::vector<uint8_t>& params);
  void ProcessWriteScreenModeSelect(const std::vector<uint8_t>& params);
  void ProcessSpecifyDownloadRegister(const std::vector<uint8_t>& params);
  void ProcessDownloadCharacter(const std::vector<uint8_t>& params);
  void ProcessDeleteDownloadedCharacter(const std::vector<uint8_t>& params);
};
