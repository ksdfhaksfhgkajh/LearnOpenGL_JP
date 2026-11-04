# 第11章：uniform と C++ からの値の設定

[目次](../README.md) · [前の章](10_glsl_in_out_and_colors.md) · [次の章](12_textures.md) · [早見表](reference.md)

## 実習：C++ から図形全体の明るさを変える

### なぜ uniform を使うか

頂点色は頂点ごとに異なるデータだった。一方、図形全体の明るさや移動量のように、一回の描画で共通に使う設定値もある。こうした値を C++ からシェーダーへ渡すために `uniform` を使う。

| 今回の入力 | 渡し方と値の違い |
| --- | --- |
| 頂点シェーダーの `in aColor` | VBO・VAO を通して頂点ごとの色を渡す |
| フラグメントシェーダーの `in vertexColor` | 頂点側の出力を、場所に応じて補間して受け取る |
| フラグメントシェーダーの `uniform float brightness` | C++ で設定し、一回の描画中は各フラグメントで共通の明るさを使う |

例えばゲーム中の状態に応じて、同じ頂点データと同じシェーダーを使い、ある図形を通常の明るさ、別の図形を暗く描ける。それぞれの描画命令の前に uniform の値を変える。

uniform はシェーダー側からは読み取り専用だが、C++ 側では描画の間に変更できる。設定した値は、その OpenGL プログラムに保存される。

### 1. フラグメントシェーダーに明るさを追加する

`shaders/fragment.glsl` を次のようにする。

```glsl
#version 330 core

out vec4 fragColor;
in vec3 vertexColor;
uniform float brightness;

void main() {
    fragColor = vec4(vertexColor * brightness, 1.0);
}
```

| 変更箇所 | 意味 |
| --- | --- |
| `uniform float brightness;` | C++ から設定する一成分の float を、`main()` の外で宣言する |
| `vertexColor * brightness` | 補間された RGB の各成分に同じ係数を掛ける |
| `vec4(..., 1.0)` | 計算した RGB にアルファ 1.0 を補って出力する |

例えば `(1.0, 0.5, 0.0) * 0.25` は `(0.25, 0.125, 0.0)`。今回は色の成分値を小さくすることで、図形を暗く見せる。

現在の `CMakeLists.txt` は `file(COPY ...)` により、CMake の設定時に `shaders/` をビルド先へコピーする。ビルドディレクトリから実行する場合はそのコピーを読むため、GLSL の編集後に CMake を再読み込みして、コピーも更新してから実行する。

### 2. C++ から値を設定する

固定値で試す場合は、`Shader` を作成し、`shaderProgram.use()` の直後に `setFloat()` を呼ぶ。

```cpp
const Shader shaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
shaderProgram.use();
shaderProgram.setFloat("brightness", 0.25f);
```

現在の `Shader` クラスには `setFloat()` が実装されている。第一引数は GLSL の uniform の名前、第二引数は渡す値。`brightness` の綴りと大文字・小文字を GLSL と一致させる。

`use()` は設定先の OpenGL プログラムを選ぶ。このクラスの `setFloat()` はその選択を前提としているため、`use()` → `setFloat()` → 描画、の順にする。

今回は固定値なので、ループに入る前に一度設定する。値は変更するかプログラムを再リンクするまで保持される。

### 3. setFloat の中の二つの関数

現在の実装が行っている処理を二段階で表すと、次のようになる。

```cpp
GLint location = glGetUniformLocation(this->mProgramID, name.c_str());
glUniform1f(location, value);
```

| 関数 | 引数と役割 | 戻り値 |
| --- | --- | --- |
| `glGetUniformLocation(program, name)` | リンク済みのプログラムを指定し、名前に対応する有効な uniform の location を調べる | location を表す `GLint`。見つからなければ `-1` |
| `glUniform1f(location, value)` | 現在 `glUseProgram()` で選ばれているプログラムの、その location に float を一つ設定する | なし |

`location` は、このプログラム内の uniform を識別する番号。VAO の属性番号やメモリーのアドレスとは別で、別のプログラムの同名 uniform が同じ番号になるとは限らない。

`glUniform1f` の `1` は一成分、`f` は float を表す。クラスにある `setInt()` と `setBool()` は、整数を渡す `glUniform1i()` を使っている。bool 用の uniform は 0 が偽、それ以外が真として扱われる。

### 値が反映されないとき

- 今回の `uniform float brightness;` には初期値を指定していないため、リンク成功直後の値は 0。C++ から設定し忘れると、図形の RGB がすべて 0 になり、黒く表示される。
- `glGetUniformLocation()` が `-1` なら、名前や、シェーダー内で実際に使っているかを確認する。出力に影響しない uniform は最適化で除かれる場合がある。
- location が `-1` の `glUniform1f()` は設定を無視する。現在の `setFloat()` は、この状況で自動的に例外を投げる実装にはなっていない。
- `glGetUniformLocation()` に指定したプログラムと、`use()` で選んだプログラムをそろえる。

### 固定値での確認（比較用）

- [x] `fragment.glsl` に uniform の宣言と RGB の乗算を追加する。
- [ ] GLSL の変更後に CMake を再読み込みし、ビルド先のシェーダーも更新する。
- [ ] 固定値を試す場合は、`use()` の直後に `setFloat()` を追加する。
- [ ] C++ の値を `1.0f` にして再ビルド・実行し、これまでの色を確認する。
- [ ] `0.25f` では図形全体が暗く、`0.0f` では黒くなることを確認する。
- [ ] 頂点色と uniform が、それぞれ何のための入力か説明できる。

2026-11-04：頂点色の表示成功を受けて、この実習を開始。uniform の追加・値の設定・表示結果は未確認。

## 実習：時間に応じて明るさを変える

2026-11-04：学習者が固定値の設定から進め、描画ループで時間に応じて uniform を更新するコードを追加した。

### 毎フレームの処理

現在のループでは、描画命令の前に次の処理を行っている。

```cpp
const GLfloat timeValue = glfwGetTime();
const GLfloat brightness = std::sin(timeValue) / 2.0f + 0.5f;
shaderProgram.setFloat("brightness", brightness);
```

時間を取得する → 明るさを計算する → uniform に設定する → `glDrawElements()` で描画する、という順序で正しい。

`std::sin()` を使う `main.cpp` では、`#include <cmath>` を明示する。現在の環境では構文確認が通ったが、使用する数学関数のヘッダーを直接読み込む。

| 処理 | 意味 |
| --- | --- |
| `glfwGetTime()` | GLFW の初期化から経過した秒数を取得する。引数なし、戻り値は `double`。このコードではタイマーの基準時刻を変更していない |
| `std::sin(timeValue)` | 時間に応じて −1～1 の間を滑らかに往復する値を計算する。引数の角度はラジアン |
| `setFloat("brightness", brightness)` | そのフレームの描画で共通に使う明るさを、現在選んでいるプログラムへ設定する |

時間を戻り値と同じ `double` で保持し、uniform に渡すときに `GLfloat` へ変換する書き方も使える。

```cpp
const double timeValue = glfwGetTime();
const GLfloat brightness =
    static_cast<GLfloat>(std::sin(timeValue) * 0.5 + 0.5);
shaderProgram.setFloat("brightness", brightness);
```

### sin の範囲を明るさへ対応付ける

| 式 | 値の範囲 |
| --- | --- |
| `sin(t)` | −1～1 |
| `sin(t) / 2` | −0.5～0.5 |
| `sin(t) / 2 + 0.5` | 0～1 |

これで黒から元の色まで、滑らかに明るくなったり暗くなったりする。`sin(t)` の周期は `2π` なので、t に秒数をそのまま使う現在の式では、約 6.28 秒で一周する。

`sin(t * 2.0)` にすると変化は二倍速になり、周期は約 3.14 秒になる。現在のコードは `sin(3.0f * timeValue)` に変更されており、周期は約 2.09 秒。フレーム数ではなく経過秒数で計算するため、同じ経過時間なら同じ明るさになる。

uniform は一回の描画中に共通の値として使われる。毎フレーム設定し直すことで、描画と描画の間にその値を変えられる。現在は使用するプログラムが一つで選択も保持されているため、ループ前の `use()` で対応できている。

### Shader のデストラクターとコンテキストの寿命

現在は `Shader::~Shader()` が `glDeleteProgram()` を呼ぶ実装になった。一方、`shaderProgram` は `main()` のスコープに置かれているため、そのままでは `glfwTerminate()` の後にデストラクターが実行される。

OpenGL のリソースは、対応するコンテキストが有効な間に削除する。今回は `Shader` の作成から描画ループ・VAO/VBO/EBO の削除までを一つのブロックに入れ、その後で GLFW を終了する。

```cpp
// GLFW・コンテキスト・GLAD の初期化後
{
    const Shader shaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    shaderProgram.use();

    // 既存の視口設定、VBO・VAO・EBO の準備
    // 既存の描画ループ（時間の取得・uniform の更新を含む）

    glUseProgram(0);
    // 既存の VBO・EBO・VAO の削除
} // ここで ~Shader() → glDeleteProgram()

glfwTerminate();
```

2026-11-04：`Shader` の作成、描画ループ、VBO・EBO・VAO の削除が `try` ブロック内に移されたことを確認。正常にブロックを抜けると `~Shader()` が実行され、その後で `glfwTerminate()` が呼ばれる。構築済みの `Shader` がある状態で例外が外へ伝わる場合も、ブロックから抜ける際にデストラクターが実行される。

### 時間変化の確認

- [x] `brightness` をフラグメント側で RGB に掛ける。
- [x] 毎フレーム、時間の取得・明るさの計算・uniform の設定を描画前に行う。
- [x] `Shader` を描画ループから使えるスコープに配置する。
- [x] `main.cpp` に `<cmath>` を明示する。
- [x] `~Shader()` が `glfwTerminate()` より先に実行されるようにする。
- [ ] 実行し、明るさの滑らかな変化と正常終了を確認する。

修正後の `main.cpp` と `Shader.cpp` は C++17 の構文確認に成功。コード上で終了順序の修正を確認し、第12章へ進む。時間変化と終了時の実際の挙動の詳細は未報告。

参考：[GLFW：時間の取得](https://www.glfw.org/docs/3.4/input_guide.html#time)、[GLFW：終了処理](https://www.glfw.org/docs/3.4/intro_guide.html#intro_terminate)。

参考：[LearnOpenGL：Shaders](https://learnopengl.com/Getting-started/Shaders)、[glGetUniformLocation](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetUniformLocation.xhtml)、[glUniform](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glUniform.xhtml)。

---

[目次](../README.md) · [前の章](10_glsl_in_out_and_colors.md) · [次の章](12_textures.md) · [早見表](reference.md)
