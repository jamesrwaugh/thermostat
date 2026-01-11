#pragma once

#include <sys/types.h>

#include <array>
#include <cstdint>
#include <istream>
#include <span>
#include <vector>

class SimGu7000Real {
 public:
  // Display dimensions
  static constexpr uint16_t DISPLAY_WIDTH = 112;
  static constexpr uint16_t DISPLAY_HEIGHT = 16;

  typedef std::array<std::array<bool, DISPLAY_HEIGHT>, DISPLAY_WIDTH>
      DisplayMemory;

  SimGu7000Real();

  // Command processing
  void ProcessCommand(const std::vector<uint8_t>& command);

  // Display memory access
  const DisplayMemory& GetDisplayMemory() const;

 private:
  typedef std::span<const uint8_t, 7> FontCharacter;

  // Font dimensions (5x7)
  static constexpr uint8_t FONT_WIDTH = 5;
  static constexpr uint8_t FONT_HEIGHT = 7;

  // Some command constants
  static constexpr uint8_t CMD_CHARACTER_DISPLAY_START = 0x20;
  static constexpr uint8_t CMD_CHARACTER_DISPLAY_END = 0xFF;
  static constexpr uint8_t CMD_BACKSPACE = 0x08;
  static constexpr uint8_t CMD_HORIZONTAL_TAB = 0x09;
  static constexpr uint8_t CMD_LINE_FEED = 0x0A;
  static constexpr uint8_t CMD_HOME_POSITION = 0x0B;
  static constexpr uint8_t CMD_CARRIAGE_RETURN = 0x0D;
  static constexpr uint8_t CMD_DISPLAY_CLEAR = 0x0C;

  // Display memory: 112x16 pixels as bool array
  // Each element represents one pixel
  DisplayMemory display_memory_;

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
  std::vector<uint8_t> command_buffer_;

  // Display memory access
  void ClearDisplayMemory();

  // Cursor management
  void SetCursor(uint16_t x, uint16_t y);
  uint16_t GetCursorX() const;
  uint16_t GetCursorY() const;

  // Character rendering
  void DrawCharacter(uint8_t character);
  void DrawCharacterAt(uint16_t x, uint16_t y, uint8_t character);

  // Helper methods
  void SetPixel(uint16_t x, uint16_t y, bool on);
  bool GetPixel(uint16_t x, uint16_t y) const;
  void DrawFontCharacter(uint16_t x, uint16_t y, uint8_t character);
  void DrawImage(uint16_t x, uint16_t y, const FontCharacter& image,
                 uint8_t width, uint8_t height);
  FontCharacter GetFontData(uint8_t character) const;

  typedef std::basic_iostream<uint8_t> Stream;

  // Top-level command implementations
  void ProcessSingleByteCommand(uint8_t first_byte);
  void ProcessEscCommand(Stream& command);
  void ProcessUsExtendedCommands(Stream& command);
  void ProcessUsCommand(Stream& command);

  // Command implementations
  void ProcessCharacterDisplay(uint8_t character);
  void ProcessBackspace();
  void ProcessHorizontalTab();
  void ProcessLineFeed();
  void ProcessHomePosition();
  void ProcessCarriageReturn();
  void ProcessDisplayClear();
  void ProcessInitializeDisplay();
  void ProcessCursorSet(Stream& params);
  void ProcessDisplayClearCommand();
  void ProcessBrightnessControl(Stream& params);
  void ProcessReverseDisplay(Stream& params);
  void ProcessHorizontalScrollSpeed(Stream& params);
  void ProcessCompositionMode(Stream& params);
  void ProcessInternationalFontSet(Stream& params);
  void ProcessCharacterCodeType(Stream& params);
  void ProcessOverwriteMode(Stream& params);
  void ProcessVerticalScrollMode(Stream& params);
  void ProcessHorizontalScrollMode(Stream& params);
  void ProcessWait(Stream& params);
  void ProcessScrollDisplayAction(Stream& params);
  void ProcessDisplayBlink(Stream& params);
  void ProcessScreenSaver(Stream& params);
  void ProcessRealTimeBitImageDisplayXy(Stream& params);
  void ProcessRealTimeBitImageDisplay(Stream& params, uint8_t x, uint8_t y);
  void ProcessCharacterFontWidthAndSpace(Stream& params);
  void ProcessFontMagnificationSet(Stream& params);
  void ProcessCurrentWindowSelect(Stream& params);
  void ProcessUserWindowDefinitionCancel(Stream& params);
  void ProcessWriteScreenModeSelect(Stream& params);
  void ProcessSpecifyDownloadRegister(Stream& params);
  void ProcessDownloadCharacter(Stream& params);
  void ProcessDeleteDownloadedCharacter(Stream& params);

  // Utilities
  void ExtractXY(Stream& s, uint8_t& x, uint8_t& y);
};
