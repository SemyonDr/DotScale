**Class**

Pascal

```
class VideoDecoder {
}
```

**Enum class**

Pascal for name and items

```
enum class ConnectionType {
    Sustained,
    Temporary
}
```

| Element                              | Naming convention            | Example                             |
| ------------------------------------ | ---------------------------- | ----------------------------------- |
| Classes                              | `PascalCase`                 | `ImageBuffer`, `JpegReader`         |
| Structs                              | `PascalCase`                 | `ScaleOptions`, `QuantizationTable` |
| Enums                                | `PascalCase`                 | `PixelLayout`, `ScaleMode`          |
| Enum values                          | `PascalCase`                 | `ScaleMode::Relative`               |
| Functions                            | `snake_case`                 | `rgb_to_gray()`, `load_image()`     |
| Methods                              | `snake_case`                 | `insert_at()`, `reshape()`          |
| Getters                              | `get_snake_case`             | `get_width()`, `get_layout()`       |
| Setters                              | `set_snake_case`             | `set_width()`, `set_layout()`       |
| Function parameters                  | `snake_case`                 | `target_width`, `row_count`         |
| Local variables                      | `snake_case`                 | `block_size`, `pixel_count`         |
| Private member variables             | `_snake_case`                | `_width`, `_layout`                 |
| Public member variables              | `snake_case`                 | `width`, `height`                   |
| Constants (`const`)                  | `snake_case`                 | `max_width`                         |
| Compile-time constants (`constexpr`) | `snake_case`                 | `rows_per_block`                    |
| Namespaces                           | `snake_case`                 | `dotscale::helper`                  |
| Template type parameters             | `PascalCase`                 | `T`, `PixelType`                    |
| Template value parameters            | `PascalCase` or `snake_case` | `TMin`, `TMax`, `rgb_to_g`          |
| Type aliases (`using`)               | `PascalCase`                 | `ImageBuffer8`, `PixelValue`        |
| Macros (if necessary)                | `UPPER_SNAKE_CASE`           | `DOTSCALE_VERSION`                  |

General rule: Use `PascalCase` for types, `snake_case` for functions and variables, and `_snake_case` for private member variables.
