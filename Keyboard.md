## Keyboard support

DCHDMI firmwares v2.3.5 and v3.0.5 added support for a dreamcast keyboard. Currently only US and German keyboard layout is supported, and it has to be configured using the web console `setup` command (default is `us`).

The keyboard has to be connected to dreamcast port A.

### Keyboard mapping

| Key (combination)    | Function/Controller Mapping                        |
|----------------------|----------------------------------------------------|
| `CTRL`+`SHIFT`+`ESC` | Buttons `L`+`R`+`Start`+`X`+`A` (open DCHDMI menu) |
| `ESC`                | Button `L` (cancel/back)                           |
| `RETURN`             | Button `R` (ok/save/confirm)                       |
| `F11`                | Button `X`                                         |
| `F12`                | Button `Y`                                         |
| `LEFT`               | D-pad `LEFT`                                       |
| `RIGHT`              | D-pad `RIGHT`                                      |
| `UP`                 | D-pad `UP`                                         |
| `DOWN`               | D-pad `DOWN`                                       |

When entering WiFi SSID/password only 7bit ASCII is supported.<br>
The following keys are working as expected:

| Key         | Function                                               |
|-------------|--------------------------------------------------------|
| `HOME`      | place cursor at the beginning of string                |
| `END`       | place cursor at the end of string                      |
| `DELETE`    | delete character at the current cursor position        |
| `BACKSPACE` | delete character preceding the current cursor position |
| `LEFT`      | move cursor to the left                                |
| `RIGHT`     | move cursor to the right                               |
