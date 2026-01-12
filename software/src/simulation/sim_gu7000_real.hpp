#pragma once

#include <sys/types.h>

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

class SimGu7000Real {
 public:
  static constexpr uint16_t DISPLAY_WIDTH = 112;
  static constexpr uint16_t DISPLAY_HEIGHT = 16;

  typedef std::array<std::array<bool, DISPLAY_HEIGHT>, DISPLAY_WIDTH>
      DisplayMemory;

  SimGu7000Real();
  void ProcessCommand(uint8_t command);
  const DisplayMemory& GetDisplayMemory() const;

 private:
  typedef std::span<const uint8_t, 7> FontCharSpan;

  // Font dimensions (5x7)
  static constexpr uint8_t FONT_WIDTH = 5;
  static constexpr uint8_t FONT_HEIGHT = 7;

  // State
  enum class State {
    Idle,
    GettingCommand,
    GettingCommandArguments,
    GettingVariableArgs,
  };

  // Some command constants
  static constexpr uint8_t CMD_CHARACTER_DISPLAY_START = 0x20;
  static constexpr uint8_t CMD_CHARACTER_DISPLAY_END = 0xFF;
  static constexpr std::array<uint8_t, 2> init_command_ = {0x1b, 0x40};

  // Display memory: 112x16 pixels as bool array
  // Each element represents one pixel
  DisplayMemory display_memory_;

  // Cursor position (in pixels)
  uint16_t cursor_x_;
  uint16_t cursor_y_;

  // Font and display settings
  State state_;
  std::vector<uint8_t> commandBytes_;
  bool initialized_;
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
  std::string command_buffer_;
  std::vector<uint8_t> command_arguments_;

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
  void DrawImage(uint16_t x, uint16_t y, const FontCharSpan& image,
                 uint8_t width, uint8_t height);
  FontCharSpan GetFontData(uint8_t character) const;

  // Command prosessing state
  typedef std::vector<uint8_t> Stream;

  typedef void (SimGu7000Real::*CommandFunction)(Stream&);
  typedef uint8_t (SimGu7000Real::*SizeGetFnFunction)();

  struct CommandItem {
    const char* const Prefix;
    const CommandFunction Execute;
    const SizeGetFnFunction SizeGetFn;
    const uint8_t FixedArgumentBytes;
  };

  static constexpr uint8_t COMMAND_COUNT = 30;

  typedef std::array<CommandItem, COMMAND_COUNT> CommandTableA;

  static CommandTableA CommandTable;
  CommandItem* CurrentCommand_;
  uint8_t CurrentCommandVariableBytes_;

  void ExecuteCurrentCommandAndReset();
  void ResetCommandState();

  // Command implementations
  void ProcessCharacterDisplay(uint8_t character);
  void ProcessBackspace(Stream& params);
  void ProcessHorizontalTab(Stream& params);
  void ProcessLineFeed(Stream& params);
  void ProcessHomePosition(Stream& params);
  void ProcessCarriageReturn(Stream& params);
  void ProcessDisplayClear(Stream& params);
  void ProcessInitializeDisplay(Stream& params);
  void ProcessCursorSet(Stream& params);
  void ProcessDisplayClearCommand(Stream& params);
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

  // Command Sizes
  uint8_t ProcessRealTimeBitImageDisplaySize();

  // Utilities
  void ExtractXY(Stream& s, uint8_t& x, uint8_t& y);
};
