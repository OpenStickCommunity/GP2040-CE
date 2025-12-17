# Keyboard Mapping Profile Implementation Plan

## 概述 / Overview

为 GP2040-CE 固件添加键盘映射 Profile 支持。键盘映射将与 GPIO Profile 绑定，切换 Profile 时（通过热键或 Web UI）键盘映射会一起切换。

Add keyboard mapping profile support to GP2040-CE firmware. Keyboard mappings will be bound to GPIO Profiles - when switching profiles (via hotkey or web UI), keyboard mappings will switch together.

## 用户需求 / User Requirements

- 4 个键盘映射档案（与 GPIO Profile 数量一致）
- 支持热键切换（使用现有的 Profile 热键）
- 绑定到 GPIO Profile（一起切换）

## 需要修改的文件 / Files to Modify

### 1. Proto 定义 / Proto Definition
**文件 / File:** `proto/config.proto`

在 `GpioMappings` 消息中添加 `keyboardMapping` 字段（第 201-206 行）：

```protobuf
message GpioMappings
{
    repeated GpioMappingInfo pins = 1 [(nanopb).max_count = 30];
    optional string profileLabel = 2 [(nanopb).max_length = 16];
    optional bool enabled = 3 [default = false];
    optional KeyboardMapping keyboardMapping = 4;  // NEW: 每个 Profile 的键盘映射
}
```

这样每个替代 Profile（2-6）都可以有自己的键盘映射。
Profile 1（基础）将继续使用 `config.keyboardMapping`。

### 2. 存储管理器 / Storage Manager
**文件 / File:** `headers/storagemanager.h`

修改 `getKeyboardMapping()` 使其支持 Profile：
```cpp
KeyboardMapping& getKeyboardMapping();
KeyboardMapping& getProfileKeyboardMapping(uint32_t profileNum);
```

**文件 / File:** `src/storagemanager.cpp`

更新 `getKeyboardMapping()` 根据当前 Profile 返回映射：
```cpp
KeyboardMapping& Storage::getKeyboardMapping() {
    uint32_t profileNum = config.gamepadOptions.profileNumber;
    if (profileNum >= 2 && profileNum <= config.profileOptions.gpioMappingsSets_count + 1) {
        GpioMappings& profile = config.profileOptions.gpioMappingsSets[profileNum - 2];
        if (profile.enabled && profile.has_keyboardMapping) {
            return profile.keyboardMapping;
        }
    }
    // 回退到基础键盘映射
    return config.keyboardMapping;
}
```

### 3. Web 配置后端 / Web Config Backend
**文件 / File:** `src/webconfig.cpp`

#### 更新 getProfileOptions（约第 591-640 行）：
在 Profile JSON 响应中添加键盘映射：
```cpp
// 对于 gpioMappingsSets 中的每个 profile
writeDoc(profileJson, "keyboardMapping", {
    {"Up", profile.keyboardMapping.keyDpadUp},
    {"Down", profile.keyboardMapping.keyDpadDown},
    // ... 所有 32 个按键
});
```

#### 更新 setProfileOptions（约第 547-589 行）：
从 Profile JSON 解析键盘映射：
```cpp
if (doc.containsKey("keyboardMapping")) {
    JsonObject kbMap = doc["keyboardMapping"];
    readDoc(profile.keyboardMapping.keyDpadUp, kbMap, "Up");
    // ... 所有 32 个按键
}
```

### 4. 前端 - Profile Store
**文件 / File:** `www/src/Store/useProfilesStore.ts`

更新 Profile 类型以包含键盘映射：
```typescript
interface Profile {
    pins: PinsType;
    profileLabel: string;
    enabled: boolean;
    keyboardMapping: KeyboardMappingType;  // NEW
}
```

### 5. 前端 - 设置页面
**文件 / File:** `www/src/Pages/SettingsPage.jsx`

修改 `keyboardModeSpecifics()` 函数（第 829-856 行）：
1. 在键盘映射表格上方添加 Profile 选择器下拉框
2. 为选定的 Profile 加载/保存键盘映射
3. 显示当前活动 Profile 指示器

```jsx
const keyboardModeSpecifics = (values, errors, setFieldValue, handleChange) => {
    return (
        <div>
            <Row className="mb-3">
                <Col sm={6}>
                    <div className="fs-3 fw-bold">{t('SettingsPage:keyboard-mapping-header-text')}</div>
                </Col>
            </Row>
            {/* NEW: Profile 选择器 */}
            <Row className="mb-3">
                <Col sm={4}>
                    <Form.Label>{t('SettingsPage:keyboard-profile-label')}</Form.Label>
                    <Form.Select
                        value={selectedKeyboardProfile}
                        onChange={(e) => handleProfileChange(e.target.value)}
                    >
                        {profiles.map((profile, index) => (
                            <option key={index} value={index + 1} disabled={!profile.enabled}>
                                {profile.profileLabel || `Profile ${index + 1}`}
                            </option>
                        ))}
                    </Form.Select>
                </Col>
            </Row>
            <Row className="mb-3">
                <Col sm={6}>
                    <div>{t('SettingsPage:keyboard-mapping-sub-header-text')}</div>
                </Col>
            </Row>
            <KeyboardMapper
                buttonLabels={buttonLabels}
                handleKeyChange={handleKeyChange}
                getKeyMappingForButton={getKeyMappingForButton}
            />
        </div>
    );
};
```

### 6. 前端 - WebApi
**文件 / File:** `www/src/Services/WebApi.js`

更新 `getKeyMappings()` 以接受 Profile 编号：
```javascript
async function getKeyMappings(setLoading, profileNum = 1) {
    // 加载特定 Profile 的键盘映射
}
```

更新 `setKeyMappings()` 以保存到特定 Profile：
```javascript
async function setKeyMappings(mappings, profileNum = 1) {
    // 保存键盘映射到特定 Profile
}
```

### 7. 本地化 / Localization
**文件 / File:** `www/src/Locales/en/SettingsPage.jsx`（及其他语言文件）

添加新的翻译键：
```javascript
'keyboard-profile-label': 'Keyboard Profile',
'keyboard-profile-current': 'Current: {{profile}}',
```

中文翻译：
```javascript
'keyboard-profile-label': '键盘档案',
'keyboard-profile-current': '当前: {{profile}}',
```

## 实现顺序 / Implementation Order

1. **Proto 修改** - 在 GpioMappings 中添加 keyboardMapping
2. **重新生成 Proto** - 运行 proto 编译
3. **存储层** - 更新 getKeyboardMapping() 使其支持 Profile
4. **Web 配置后端** - 在 Profile API 中添加键盘映射
5. **前端 Store** - 更新 Profile 类型定义
6. **前端 UI** - 在键盘映射部分添加 Profile 选择器
7. **本地化** - 添加翻译字符串
8. **测试** - 验证 Profile 切换正常工作

## 迁移策略 / Migration Strategy

现有配置将继续工作：
- Profile 1 使用现有的 `config.keyboardMapping`（不变）
- Profile 2-6 初始时键盘映射为空
- 空键盘映射会回退到 Profile 1 的映射
- 用户可以选择性地为每个 Profile 配置键盘映射

## 测试清单 / Testing Checklist

- [ ] Profile 1 键盘映射正常工作（向后兼容）
- [ ] Profile 2-6 可以有自定义键盘映射
- [ ] 通过热键切换 Profile 会改变键盘映射
- [ ] 通过 Web UI 切换 Profile 会改变键盘映射
- [ ] 空的 Profile 键盘映射会回退到 Profile 1
- [ ] Web 配置正确保存/加载 Profile 键盘映射
- [ ] 键盘驱动使用基于当前 Profile 的正确映射

## 注意事项 / Notes

- 热键切换已经存在（下一个/上一个 Profile 热键）
- 不需要新的热键动作 - 键盘映射随 GPIO Profile 切换
- Profile 启用/禁用仍由 GPIO Profile 设置控制
- 最多 6 个 Profile（1 个基础 + 5 个替代）- 与现有系统一致

## 关键文件路径 / Key File Paths

```
proto/config.proto                      # Proto 定义
headers/storagemanager.h                # 存储管理器头文件
src/storagemanager.cpp                  # 存储管理器实现
src/webconfig.cpp                       # Web 配置后端
src/drivers/keyboard/KeyboardDriver.cpp # 键盘驱动（使用 getKeyboardMapping）
www/src/Pages/SettingsPage.jsx          # 设置页面 UI
www/src/Store/useProfilesStore.ts       # Profile 状态管理
www/src/Services/WebApi.js              # API 调用
www/src/Locales/*/SettingsPage.jsx      # 本地化文件
```
