# 第8章：EBO とインデックス描画

[目次](../README.md) · [前の章](07_fragment_shader_and_program.md) · [次の章](09_wireframe_and_input.md) · [早見表](reference.md)

## 実習：EBO で頂点を共有し、四角形を描く

### なぜ EBO を使うか

四角形を二つの三角形で作る。各三角形の三頂点を順番に保存すると、合わせて六頂点分のデータが必要になり、共有する角のデータが重複する。

そこで、頂点データを四つ保存し、「どの頂点をどの順番で使うか」を別の整数配列で指定する。この描き方をインデックス描画という。EBO は Element Buffer Object の略で、この頂点番号（インデックス）を保存するバッファ。

```text
3 -------- 2
|       / |
|     /   |
|   /     |
| /       |
0 -------- 1

一つ目の三角形：0 → 1 → 2
二つ目の三角形：2 → 3 → 0
```

頂点 0 と 2 のデータを、両方の三角形が共有する。VBO は「各頂点が持つ値」、EBO は「使う頂点の番号と順番」を保持する。複雑なメッシュでも、同じ頂点を複数の三角形から参照できる。

EBO も通常のバッファオブジェクトなので、作成・転送・削除には、VBO で使った `glGenBuffers()`・`glBufferData()`・`glDeleteBuffers()` を使う。

### 1. 頂点を四つにし、インデックスを追加する

ファイル上部の `vertices` を以下に置き換え、その直後に `indices` を追加する。

```cpp
static constexpr float vertices[] = {
    -0.5f, -0.5f, 0.0f,  // 0：左下
     0.5f, -0.5f, 0.0f,  // 1：右下
     0.5f,  0.5f, 0.0f,  // 2：右上
    -0.5f,  0.5f, 0.0f   // 3：左上
};

static constexpr GLuint indices[] = {
    0, 1, 2,
    2, 3, 0
};
```

`indices` の値は頂点番号。属性番号・VAO の ID・VBO の ID・float 配列の要素番号ではない。例えば頂点番号 2 は、三つ目の頂点 `(0.5, 0.5, 0.0)` を指す。

今回は一頂点が三つの float という形式が同じなので、属性 0 の読み方は `3 * sizeof(float)` のまま使う。既存の `glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW)` は、変更後の配列全体を VBO にコピーする。

### 2. EBO を作成して、インデックスをコピーする

`glEnableVertexAttribArray(0)` の後、`while` の前に追加する。この時点では、初期化時に選んだ `vao` がバインドされている。

```cpp
GLuint ebo = 0;
glGenBuffers(1, &ebo);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
```

| 行 | 役割 |
| --- | --- |
| `GLuint ebo = 0;` | EBO の ID を受け取る変数 |
| `glGenBuffers(1, &ebo)` | バッファの ID を一つ取得する |
| `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)` | 現在の VAO が使うインデックスバッファとして ebo を選ぶ |
| `glBufferData(...)` | 選んだ EBO に格納領域を確保し、六つの頂点番号をコピーする |

`GL_ELEMENT_ARRAY_BUFFER` は、インデックス描画で頂点番号を読み出すためのバッファのバインド先。VBO で使った `GL_ARRAY_BUFFER` とは、VAO への記録のタイミングが異なる。

| 操作 | VAO に保存されるタイミング |
| --- | --- |
| `GL_ARRAY_BUFFER` の VBO を属性の読み出し元にする | `glVertexAttribPointer()` が、選択中の VBO への参照をその属性に記録する |
| `GL_ELEMENT_ARRAY_BUFFER` の EBO を選ぶ | `glBindBuffer()` 自体が、現在の VAO に EBO への参照を記録する |

Core Profile では、EBO のバインドを設定する前に、作成した VAO を選んでおく。保存した VAO を描画時に選ぶと、その EBO も選ばれる。現在のように一つの VAO を選択したまま使うなら、ループ内で EBO を毎回バインドする必要はない。

同じ VAO を選んだ状態で別の EBO をバインドすると、VAO の EBO への参照が変わる。`glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)` も、その VAO の EBO の参照を解除する操作になる。

### 3. 描画命令を glDrawElements に変更する

ループ内の既存の `glDrawArrays(GL_TRIANGLES, 0, 3)` を、次の一行に置き換える。

```cpp
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
```

| 引数 | 今回の意味 |
| --- | --- |
| `GL_TRIANGLES` | 三つのインデックスごとに一つの三角形を作る |
| `6` | EBO から読むインデックスの個数。保存した四頂点の数とは別で、`0, 1, 2, 2, 3, 0` の六個を読む |
| `GL_UNSIGNED_INT` | インデックスの型。今回の `GLuint` の配列に対応する。頂点の位置データの型は `GL_FLOAT` |
| `nullptr` | EBO の先頭から読む。バイト単位の開始位置（offset）が 0 |

`nullptr` は CPU 側の `indices` 配列のアドレスを渡す意味ではない。現在の VAO の EBO にコピー済みのインデックスを、その先頭から使う指定。

```text
EBO：0, 1, 2, 2, 3, 0
     ↓  ↓  ↓  ↓  ↓  ↓
VBO：頂点番号に対応する各属性を読み出す
     → 三頂点ずつで二つの三角形を描く
```

### 4. EBO の後片付けを追加する

ループ後、`glDeleteVertexArrays(1, &vao)` の後、既存の VBO の削除より前に次を追加する。

```cpp
glDeleteBuffers(1, &ebo);
```

これも `glfwTerminate()` より前に行う。

### 共有する頂点とデータ量について

共有するのは、位置・色など、その頂点に与える全属性が同じ組み合わせ。同じ位置でも、異なる色などを与えたいときは別の頂点として扱うことがある。

インデックス自体にも格納領域が必要なので、EBO を使えば必ず合計のデータ量が減るわけではない。今回の位置だけの例では、この環境で「六頂点 × 12 バイト」と「四頂点 × 12 バイト + 六インデックス × 4 バイト」は、どちらも 72 バイト。頂点が多く共有されるモデルや、一頂点に位置・色など多くの属性がある場合に、重複を省く効果が大きくなる。

### 表示されなかった原因：インデックスの型と三角形の組み方

2026-10-25 の学習者のコードでは、インデックスが `static constexpr float triangleIndices[]` として保存されていた。一方、描画命令は `glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr)` だった。

主な原因は、この型の不一致。`glBufferData()` は渡された領域のバイト列をコピーするため、float の数値を整数の頂点番号に変換する処理は行わない。`glDrawElements()` は、そのバイト列を指定されたインデックスの型として解釈する。

この環境で float の `1.0f` のビット列を符号なし 32 ビット整数として読むと、1 ではなく 1,065,353,216 になる。実際の配列のバイト列を確認すると、次の番号として解釈された。

```text
保存した float の値：0, 1, 2, 1, 2, 3
GL_UNSIGNED_INT として読む値：
    0, 1065353216, 1073741824, 1065353216, 1073741824, 1077936128
```

用意した頂点番号は 0〜3 なので、範囲外の頂点を参照する原因になる。C++ のコンパイルだけでは、この組み合わせの誤りは検出されない。

さらに、学習者の `quadVertices` は次の番号順になっていた。この順番も四角形の頂点配置として有効だが、インデックスはその配置に合わせる。

```text
3 -------- 0
|       / |
|     /   |
|   /     |
| /       |
2 -------- 1
```

元の `0, 1, 2` と `1, 2, 3` は、四角形の下辺 1–2 を共有しており、同じ側へ広がる三角形になる。型だけを直すと、重なる部分と覆われない部分が生じる。

この頂点配置では、例えば 0–2 の対角線を共有するように、配列全体を次に置き換える。

```cpp
static constexpr GLuint triangleIndices[] = {
    0, 1, 2,
    2, 3, 0
};
```

`GLuint` の整数配列と、描画時の `GL_UNSIGNED_INT` が対応する。二つの三角形は対角線 0–2 の両側を覆う。

CPU 上で確認した例として、四角形内部の点 `(0.0, 0.25)` は元の番号の組み合わせでは覆われず、`(0.0, -0.25)` は二回覆われる。修正案では、どちらも一回ずつ覆われることを確認した。その後、コード上の修正と、学習者からの四角形の表示成功の報告を確認した。

### 今回の確認

- [x] 頂点を四つにし、六要素の配列を追加する（実装名は quadVertices と triangleIndices）。
- [x] triangleIndices を GLuint の配列へ直し、対角線を共有する二つの三角形にする。
- [x] VAO が選択された状態で EBO を設定し、配列をコピーする。
- [x] glDrawArrays を glDrawElements に変更する。
- [x] 正常終了時に EBO を削除する。
- [x] オレンジ色の四角形が表示されることを確認する（2026-10-25、学習者から成功の報告）。

今回新しく使った描画 API は `glDrawElements()`。コード上の修正と表示成功の報告を確認し、この実習は完了。

参考：[LearnOpenGL：Element Buffer Objects](https://learnopengl.com/Getting-started/Hello-Triangle)、[glDrawElements](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElements.xhtml)、[glBufferData](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferData.xhtml)、[glBindBuffer](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindBuffer.xhtml)。


---

[目次](../README.md) · [前の章](07_fragment_shader_and_program.md) · [次の章](09_wireframe_and_input.md) · [早見表](reference.md)
