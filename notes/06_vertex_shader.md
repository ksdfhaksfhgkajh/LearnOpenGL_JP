# 第6章：頂点シェーダー

[目次](../README.md) · [前の章](05_vao_and_vertex_attributes.md) · [次の章](07_fragment_shader_and_program.md) · [早見表](reference.md)

## 実習：頂点シェーダーのソースを書く

### パイプラインのどこを作るか

頂点シェーダーは、各頂点の属性を受け取り、その頂点を描画に使う位置へ変換する GPU 用のプログラム。OpenGL Shading Language（GLSL）で書く。

```text
VBO の位置データ
    → VAO の属性 0 の設定に従って読み出す
    → 頂点シェーダーの入力 aPos
    → 頂点シェーダーが gl_Position にクリップ座標を出力
    → 後続のクリッピング・透視除算・視口変換などへ
```

今回使う OpenGL 3.3 Core の描画では、自分で頂点シェーダーを用意する。VBO に座標を保存し、VAO に読み方を記録した後、ここで描画用の位置を計算する手順を指定する。

現在の `vertices` は一頂点につき位置の三つの float を保持しているので、入力は属性番号 0 の `vec3` とする。まずは入力の x・y・z をそのまま使い、w に 1.0 を補って出力する。

### 追加するコード

`src/main.cpp` の `vertices` 配列の後、C++ の `int main()` より前に、自分で次の文字列を追加する。

```cpp
static const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";
```

外側は C++ の文字列。`R"(` と `)"` の間にある内容が GLSL のソースで、この後の工程で OpenGL に渡す。C++ コンパイラーはこの中身を文字列として扱うため、C++ のビルド成功だけでは GLSL の文法は確認できない。

### GLSL の各行の意味

| 行・記号 | 意味 |
| --- | --- |
| `#version 330 core` | GLSL 3.30 の Core 向けの記述を使う宣言。OpenGL 3.3 に対応する |
| `layout (location = 0)` | この入力を属性番号 0 に対応させる。`glVertexAttribPointer()` と `glEnableVertexAttribArray()` に渡した番号と一致させる |
| `in` | このシェーダーへの入力であることを示す |
| `vec3 aPos` | 三つの float を持つ入力変数。今回は一頂点の位置 x・y・z。名前は自分で決められる |
| `void main()` | このシェーダーの処理の入口。C++ の `main()` とは別のプログラムの関数で、描画時に各頂点を処理するために使われる |
| `vec4(aPos, 1.0)` | aPos の三成分に 1.0 を加えて `(x, y, z, 1.0)` の四成分を作る |
| `gl_Position` | GLSL に組み込まれた位置の出力変数。型は `vec4` で、クリップ座標 `(x, y, z, w)` を書き込む |

`gl_Position` は変数なので、`gl_Position(...)` と関数のように呼ぶのではなく、計算した値を代入する。

### w = 1.0 と NDC の関係

頂点シェーダーが出す `gl_Position` はクリップ座標。その後、クリッピングを経て x・y・z を w で割ると NDC になる。今回は w が 1.0 なので、割った後も数値がそのままになる。

例えば一つ目の頂点は次のように流れる。

```text
aPos        = (-0.5, -0.5, 0.0)
gl_Position = (-0.5, -0.5, 0.0, 1.0)
NDC         = (-0.5 / 1.0, -0.5 / 1.0, 0.0 / 1.0)
            = (-0.5, -0.5, 0.0)
```

四つ目の値 w は透視除算に使う成分。ここでは 1.0 を使い、透視投影を学ぶ段階で、座標変換との関係を詳しく扱う。

応用として、頂点シェーダー内で `vec4(aPos * 0.5, 1.0)` を出力すると、原点を中心に各座標が半分になり、三角形も半分の大きさになる。VBO の元データを変えずに、描画に使う位置を計算できる。

### 今回の到達点と次の工程

- [ ] 頂点シェーダーのソースを文字列として追加する。
- [ ] VAO の属性番号 0 と、シェーダー入力の location 0 の対応を確認する。
- [ ] 入力 `aPos` と出力 `gl_Position`、クリップ座標と NDC の違いを説明できる。

この文字列を追加しただけでは、シェーダーはまだコンパイルも実行もされず、表示は背景色のまま。続いて、ソースをシェーダーオブジェクトへ渡し、コンパイルして結果を確認する。その後、フラグメントシェーダー・プログラムのリンク・描画命令を順に用意する。

参考：[LearnOpenGL：Vertex shader](https://learnopengl.com/Getting-started/Hello-Triangle)、[GLSL 3.30 公式仕様：入力の location・ベクトルの構築・gl_Position](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.3.30.pdf)。

## 実習：頂点シェーダーをコンパイルする

### ソースの確認

GLSL の代入文は次のようにする。組み込み変数の名前は正確に `gl_Position` とし、行末に `;` を付ける。

```glsl
gl_Position = vec4(aPos, 1.0);
```

GLSL は C++ 内の文字列なので、この中の変数名や文法の誤りは、通常の C++ のビルドでは検出できない。以下の実行時のコンパイルと、結果・ログの取得で確認する。

### シェーダーオブジェクトとは

GLSL のソース、コンパイル結果、診断ログなどを保持する OpenGL オブジェクト。`GLuint` の ID で扱う。これらの API は、処理対象のシェーダーを ID の引数で直接指定する。

```text
glCreateShader   → 空の頂点シェーダーオブジェクトを作る
glShaderSource   → そのオブジェクトに GLSL の文字列をコピーする
glCompileShader  → ソースをコンパイルする
glGetShaderiv    → 成功・失敗を取得する
失敗した場合    → glGetShaderInfoLog で原因を取得する
```

コンパイルは初期化時に行う。頂点シェーダーとして各頂点を処理するのは、後でプログラムをリンクし、有効にして、描画命令を出したとき。

### 追加する場所

`main()` 内の GLAD 初期化成功後、既存の `glfwGetFramebufferSize()` より前に、以下を順番に追加する。この位置では VBO・VAO はまだ作成していないため、ここで失敗したときはシェーダーを片付け、`glfwTerminate()` を呼んで終了できる。

### 1. オブジェクトを作成する

```cpp
GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
if (vertexShader == 0) {
    std::cerr << "Failed to create vertex shader\n";
    glfwTerminate();
    return -1;
}
```

`GL_VERTEX_SHADER` は、作成するシェーダーの種類。成功すると `glCreateShader()` が ID を直接返す。まだソースもコンパイル結果もなく、作成に失敗した場合の ID は 0。

### 2. ソースを登録してコンパイルする

```cpp
glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
glCompileShader(vertexShader);
```

`glShaderSource()` の引数：

| 引数 | 今回の意味 |
| --- | --- |
| `vertexShader` | ソースの登録先のシェーダー ID |
| `1` | 渡す文字列の個数。一行という意味ではなく、複数行の GLSL を含む一つの文字列 |
| `&vertexShaderSource` | 文字列ポインターの配列を受け取る API に、一つの文字列ポインターのアドレスを渡す |
| `nullptr` | 各文字列の長さを別の配列で指定せず、文字列の終端までを使う |

この関数は文字列をシェーダーオブジェクトへコピーする段階で、ここではまだ文法は検査されない。

`glCompileShader(vertexShader)` で、登録した GLSL を OpenGL の実装にコンパイルさせる。戻り値はなく、成功・失敗はシェーダーオブジェクト側に保存される。

### 3. コンパイル結果を確認する

上の二行に続けて追加する。

```cpp
GLint compiled = GL_FALSE;
glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);

if (compiled != GL_TRUE) {
    GLchar infoLog[1024]{};
    glGetShaderInfoLog(vertexShader, 1024, nullptr, infoLog);
    std::cerr << "Vertex shader compilation failed:\n" << infoLog << '\n';
    glDeleteShader(vertexShader);
    glfwTerminate();
    return -1;
}

std::cout << "Vertex shader compilation succeeded\n";
```

`glGetShaderiv()` に `GL_COMPILE_STATUS` を渡すと、直前のコンパイルの成功なら `GL_TRUE`、失敗なら `GL_FALSE` が `compiled` に書き込まれる。戻り値で受け取る関数ではない。

失敗時の `glGetShaderInfoLog()` の引数：

| 引数 | 今回の意味 |
| --- | --- |
| `vertexShader` | ログを取得するシェーダーの ID |
| `1024` | `infoLog` に格納できる文字数。文字列の終端も含む |
| `nullptr` | 実際に取得したログの長さは使わないので、その出力先を省略する |
| `infoLog` | ログの文字列を書き込む配列 |

この例では最大 1024 文字分の領域を使うため、それより長いログは途中までになる。メッセージの文面や行番号の形式は OpenGL の実装によって異なる。

シェーダーの作成失敗と、作成済みシェーダーのコンパイル失敗は別。失敗処理の `glDeleteShader()` は、作成済みのオブジェクトを片付けるために呼ぶ。

### 正常終了時の後片付け

成功した場合に備え、`while` の後、既存の `glDeleteBuffers()` より前に次を追加する。

```cpp
glDeleteShader(vertexShader);
```

先の VAO 実習で案内した `glDeleteVertexArrays(1, &vao)` も、VBO の削除より前に追加する。いずれも現在のコンテキストがある間、`glfwTerminate()` より前に行う。

後でプログラムのリンクを学ぶ段階では、リンク後にシェーダーの削除を要求する形へ整理する。今回はコンパイルしたシェーダーを、正常終了まで保持する。

### 今回の到達点

- [ ] GLSL 内の組み込み変数名と文末を修正する。
- [ ] 頂点シェーダーを作成し、ソースを登録してコンパイルする。
- [ ] `GL_COMPILE_STATUS` を取得し、成功時にはコンソールに成功メッセージが出ることを確認する。
- [ ] 失敗時にログを表示して終了できるようにする。
- [ ] 正常終了時にシェーダーと VAO を片付ける。

この段階の表示は背景色のまま。次はフラグメントシェーダーで色を出力し、両方のシェーダーをプログラムとしてリンクする。

参考：[glCreateShader](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCreateShader.xhtml)、[glShaderSource](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glShaderSource.xhtml)、[glCompileShader](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCompileShader.xhtml)、[glGetShaderiv](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetShader.xhtml)、[glGetShaderInfoLog](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetShaderInfoLog.xhtml)、[glDeleteShader](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDeleteShader.xhtml)。


---

[目次](../README.md) · [前の章](05_vao_and_vertex_attributes.md) · [次の章](07_fragment_shader_and_program.md) · [早見表](reference.md)
