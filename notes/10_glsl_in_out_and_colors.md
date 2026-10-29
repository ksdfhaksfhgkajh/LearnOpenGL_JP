# 第10章：GLSL の入出力と頂点色

[目次](../README.md) · [前の章](09_wireframe_and_input.md) · [早見表](reference.md)

## 実習：GLSL の in・out と頂点色の補間

### 何を作るか

四つの頂点に赤・緑・青・黄を持たせる。頂点シェーダーが各頂点の色を受け取り、フラグメントシェーダーへ渡す。各フラグメントには、その位置に応じて補間された色が届くので、面にグラデーションができる。

例えば、各地点の温度を頂点の色として保存し、面上の分布を滑らかに表示する用途にも使える。

```text
VAO 属性 0 → 頂点シェーダーの in aPos   → gl_Position
VAO 属性 1 → 頂点シェーダーの in aColor → out vertexColor
                                             ↓ ラスタライズ時に補間
                              フラグメントシェーダーの in vertexColor
                                             ↓
                                         out FragColor
```

今回の変更は、頂点データ・VAO の読み方・二つの GLSL の三箇所。EBO の頂点番号は、同じ四頂点を指すのでそのまま使える。

### 1. quadVertices に色を追加する

一頂点を `x, y, z, r, g, b` の六つの float にする。ファイル上部の `quadVertices` を次に置き換える。

```cpp
static constexpr GLfloat quadVertices[] = {
    // 位置               // 色
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // 0：右上、赤
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // 1：右下、緑
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  // 2：左下、青
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f   // 3：左上、黄
};
```

既存の VBO の `glBufferData()` は `sizeof(quadVertices)` を使っているため、この配列全体をコピーする。

インデックス `0, 1, 2, 2, 3, 0` は同じものを使う。一つのインデックスは、その頂点の位置と色の両方に適用される。例えば頂点番号 2 では、位置 `(-0.5, -0.5, 0.0)` と青 `(0.0, 0.0, 1.0)` を読む。

### 2. VAO に位置と色の読み方を設定する

既存の属性 0 の設定・有効化の二行を、次の四つの呼び出しに置き換える。現在のコードでは、この位置で VAO と VBO が選択済み。

```cpp
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                      6 * sizeof(GLfloat), nullptr);
glEnableVertexAttribArray(0);

glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                      6 * sizeof(GLfloat),
                      reinterpret_cast<const void*>(3 * sizeof(GLfloat)));
glEnableVertexAttribArray(1);
```

| 属性 | 読む内容 | stride | offset |
| --- | --- | --- | --- |
| 0 | 位置 x・y・z | `6 * sizeof(GLfloat)` | 0 |
| 1 | 色 r・g・b | `6 * sizeof(GLfloat)` | `3 * sizeof(GLfloat)` |

両方とも次の頂点へ進む間隔は六つの float。この環境では stride は 24 バイト、色の offset は 12 バイト。`GLfloat` はこの環境では float の別名なので、以前の `sizeof(float)` を使う例と同じ大きさになる。

### 3. 頂点シェーダーの入力と出力を増やす

既存の `vertexShaderSource` を次に置き換える。

```cpp
static constexpr auto vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";
```

| 行 | 役割 |
| --- | --- |
| `layout (location = 1) in vec3 aColor;` | VAO の属性番号 1 から、この頂点の RGB を受け取る |
| `out vec3 vertexColor;` | 次のステージへ渡す三成分の出力変数を宣言する |
| `vertexColor = aColor;` | 受け取った頂点色を、この頂点の出力として書き込む |

`aColor` は頂点シェーダーの入力、`vertexColor` はその出力。それぞれ別の変数。頂点ごとに、赤・緑・青・黄の対応する値を出力する。

### 4. フラグメントシェーダーで色を受け取る

既存の `fragmentShaderSource` を次に置き換える。

```cpp
static constexpr auto fragmentShaderSource = R"(
#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";
```

`in vec3 vertexColor` で色を受け取り、RGB の三成分にアルファ 1.0 を加えて、最終的な色として出力する。

今回の GLSL 3.30 の書き方では、頂点側の `out vec3 vertexColor` とフラグメント側の `in vec3 vertexColor` の名前と型をそろえる。これらはプログラムのリンク時に対応付けられる。既存のコンパイル・リンク結果の確認処理も使う。

接続は二種類ある。

| 接続 | 対応させるもの |
| --- | --- |
| VAO → 頂点シェーダー | 属性番号と、入力の `layout(location = ...)` |
| 頂点シェーダー → フラグメントシェーダー | 今回は out / in の変数名・型・補間に関する指定 |

### なぜ頂点以外の場所にも色が付くか

頂点シェーダーは頂点ごとに色を出す。一方、ラスタライズで生まれる各フラグメントには、その場所に応じて頂点色を混ぜた値が渡される。この計算が補間。

今回の浮動小数点の入力は、通常の透視補正付き補間が適用される。全頂点の `gl_Position.w` が 1.0 なので、今回は画面上の位置に応じた通常の線形補間として考えられる。補間の細かな指定は後で扱う。

例えば、対角線 0–2 の中点では、赤と青を半分ずつ混ぜた値になる。

```text
頂点 0：赤 (1.0, 0.0, 0.0)
頂点 2：青 (0.0, 0.0, 1.0)
中点の補間値 = 0.5 × 赤 + 0.5 × 青
             = (0.5, 0.0, 0.5)  紫
```

補間は三角形ごとに行われる。四角形全体で四つの色を一度に平均する計算ではない。共有辺の頂点色が同じなので、二つの三角形の共有辺上では色の値がつながる。

### 今回の確認

- [ ] quadVertices に各頂点の RGB を追加する。
- [ ] 位置と色の両方で stride を六つの float にし、色の属性 1 を有効化する。
- [ ] 頂点シェーダーの色の入力・出力と、フラグメントシェーダーの色の入力を追加する。
- [ ] 両シェーダーのコンパイルと、プログラムのリンクが成功する。
- [ ] 塗りつぶし表示で、四隅の色と面のグラデーションを確認する。

色の観察には 2 キーで塗りつぶし表示を選ぶ。今回新しい OpenGL 関数はなく、既に学んだ頂点属性設定を再利用する。コードの追加・GLSL のコンパイルとリンク・色の表示結果は未確認。

参考：[LearnOpenGL：Shaders の入力・出力と頂点色](https://learnopengl.com/Getting-started/Shaders)、[GLSL 3.30 公式仕様：ステージ間の入出力・補間](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.3.30.pdf)。


---

[目次](../README.md) · [前の章](09_wireframe_and_input.md) · [早見表](reference.md)
