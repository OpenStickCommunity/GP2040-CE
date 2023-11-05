# Macro Settings

Purpose: Macros are a series or combination of gamepad inputs triggered with a single button or a combination of buttons.

![GP2040-CE Configurator - Macros](../assets/images/gpc-macros.png)

## Macro Options

`Macro Button Pin` - The GPIO Pin used to activate macros in combination with other inputs when `Uses Button` is enabled and set.

### Label

This is the name of the macro and is optional.

### Activate

- `Off` - The macro is not available for use.
- `On` - The macro is available for use when triggered by either the GPIO pin or the combination set in [`Uses Button`](#uses-button).
  
### Interruptible

- `Off` - The macro cannot be stopped and will execute all lines until the macro is completed.
- `On` - Any input will stop the current ongoing macro or, if the input is another macro, the interrupting macro trigger will start executing instead.

### Exclusive

- `Off` - Additional inputs from user will be sent as the macro continues to execute, resulting in a blend of macro and user inputs.
- `On` - Filters only the macro inputs through and excludes any additional user gamepad inputs

?> This option is **only** available when `Interruptible` is enabled.

### Show Frames

- `Off` - The duration in the macro input line editor will appear in milliseconds (ms).
- `On` - The duration in the macro input line editor will appear as a number of frames, assuming 60 frames per second.

### Uses Button

- `Off` - This macro is assigned to a GPIO pin on the board and when pressed, the macro will be triggered.
- `On` - This macro is assigned to a button combination of `Macro Button Pin + Input` where the input can be any of the standard gamepad inputs.

### Trigger Mode

This describes is how the GPIO pin or the combination set in [`Uses Button`](#uses-button) triggers and repeats the macro.

- Press - Full press of button triggers the macro once
- Hold Repeat - Holding button to repeatedly triggers macro
- Toggle - Full press of button causes the macro to repeatedly trigger, another full press of the button will stop the macro from repeatedly triggering.

## Macro Input Line Editor

Each individual macro has a maximum of 50 lines where each input line has a maximum of 18 gamepad inputs (using all available gamepad inputs).

![GP2040-CE Configurator - Macro Input Line](../assets/images/gpc-macros-input-line.png)

Each input line is composed of the following elements from left to right.

`Input Line Duration` ms `Inputs` | `Post Input Wait Duration` ms

- Input Line Duration - The duration that the inputs are held for. (Maximum 4,294,967ms or 268,435 frames)
- Inputs - The inputs to be held during the execution of the input line.
- Post Input Wait Duration - The duration in-between when that input line finishes executing and when the next line will start executing. (Maximum 4,294,967ms or 268,435 frames)

?> To delete an input line, double-click the "x" button.

## Note

- When a macro stops for any reason and are triggered. again, the macro starts again from the beginning.