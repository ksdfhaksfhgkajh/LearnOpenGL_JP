# 第7章：フラグメントシェーダーとプログラム

[目次](../README.md) · [前の章](06_vertex_shader.md) · [次の章](08_ebo_and_indexed_drawing.md) · [早見表](reference.md)

## 実習：フラグメントシェーダーで色を出力する

### 先に頂点シェーダーの結果確認を整える

ログには、失敗の原因だけでなく、成功時の警告なども入ることがある。`GL_INFO_LOG_LENGTH > 0` は「ログがある」という意味で、コンパイル失敗を表す条件ではない。成功・失敗は `GL_COMPILE_STATUS` で確認する。

現在のログの長さを取得する処理の直前に、次の二行を追加する。

```cpp
GLint compiled = GL_FALSE;
glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
```

続いて、既存の `if (infoLogLength > 0)` を `if (compiled != GL_TRUE)` に変更する。ログを取得する行も、保存先に合わせた容量に変更する。

```cpp
glGetShaderInfoLog(vertexShader, infoLogLength, nullptr, infoLog.data());
```

`std::string infoLog(infoLogLength, '\0')` で確保した領域に対して、取得容量だけを固定の 512 にすると、領域より多くの文字を書き込ませる指定になったり、長いログを途中までしか取得できなかったりする。確保した長さと取得容量を一致させる。必要容量が 0 なら、取得する文字数も 0 で、ログは空になる。

### フラグメントシェーダーの役割

三角形がラスタライズされると、画面のピクセルへ書き込む候補であるフラグメントが生成される。フラグメントシェーダーは、そのフラグメントの色などを計算するプログラム。

```text
頂点シェーダー：位置を出力
    → プリミティブの組み立て・クリッピング・ラスタライズなど
    → フラグメントシェーダー：色を出力
    → 深度テスト・ブレンディングなどを経てフレームバッファへ
```

今回は、三角形の描画で生成される各フラグメントへ同じオレンジ色を出す。後で、照明やテクスチャなどを使って場所ごとに色を変えられる。

### ソースを追加する

`src/main.cpp` の `vertexShaderSource` の後、C++ の `main()` より前に追加する。

```cpp
static const char* fragmentShaderSource = R"(
#version 330 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";
```

| 行・記号 | 意味 |
| --- | --- |
| `#version 330 core` | 頂点側と同じ GLSL 3.30 Core を使う |
| `out` | このシェーダーからの出力変数を宣言する |
| `vec4 FragColor` | 四つの float を持つ出力変数。`FragColor` は自分で付けた名前 |
| `void main()` | フラグメントを処理するときの入口 |
| `vec4(1.0, 0.5, 0.2, 1.0)` | 今回は四成分を赤・緑・青・アルファ（RGBA）として使い、オレンジ色を作る |

R・G・B は今回は 0.0〜1.0 の値で指定し、A は 1.0 とする。透明な色の合成は、後でブレンディングを学ぶときに扱う。一定の色を出力するため、今回のフラグメントシェーダーは入力変数を必要としない。

背景色は `glClearColor()` で設定し、`glClear()` でフレームバッファへ書き込む。三角形を描画するときは、その描画で生まれたフラグメントの色を、このシェーダーで指定する。

### 作成・コンパイル・結果確認を追加する

`main()` 内の頂点シェーダーの成功メッセージの後、`glfwGetFramebufferSize()` より前に追加する。使用する API の流れは頂点側と同じ。種類は `GL_FRAGMENT_SHADER`、ID は `fragmentShader`、ソースは `fragmentShaderSource` とする。

```cpp
GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
if (fragmentShader == 0) {
    std::cerr << "Failed to create fragment shader\n";
    glDeleteShader(vertexShader);
    glfwTerminate();
    return -1;
}

glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
glCompileShader(fragmentShader);

GLint fragmentCompiled = GL_FALSE;
glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentCompiled);

if (fragmentCompiled != GL_TRUE) {
    GLint fragmentLogLength = 0;
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &fragmentLogLength);
    std::string fragmentLog(fragmentLogLength, '\0');
    glGetShaderInfoLog(fragmentShader, fragmentLogLength, nullptr,
                       fragmentLog.data());
    std::cerr << "Fragment shader compilation failed:\n"
              << fragmentLog << '\n';
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
    glfwTerminate();
    return -1;
}

std::cout << "Fragment shader compilation succeeded\n";
```

`GL_FRAGMENT_SHADER` は作成するシェーダーの種類を指定する定数。`glShaderSource()` と `glCompileShader()` の役割は、頂点側で学んだものと同じ。

結果やログを取得する対象も、必ず `fragmentShader` にする。頂点側の変数と同じスコープに置くので、結果やログの長さは `fragmentCompiled`・`fragmentLogLength` という別の名前にしている。

この位置では、フラグメント側に失敗すると頂点シェーダーは既に作成済みなので、それも削除してから終了する。`<string>` は現在の `include/main.h` に追加済み。

### 正常終了時の後片付け

`while` の後、既存の頂点シェーダーの削除より前に、次を追加する。

```cpp
glDeleteShader(fragmentShader);
```

### 今回の確認と次の工程

- [ ] 頂点側の成功・失敗の判定を `GL_COMPILE_STATUS` に直す。
- [ ] ログ取得時の容量を、保存先の長さに合わせる。
- [ ] フラグメントシェーダーのソース・作成・コンパイル・結果確認を追加する。
- [ ] 実行時に、頂点側とフラグメント側の両方の成功メッセージが出ることを確認する。
- [ ] 正常終了時に両方のシェーダーを削除する。

両シェーダーのコンパイルまでの段階では表示は背景色のまま。次は、二つを一つのシェーダープログラムにリンクし、描画で使えるようにする。

参考：[LearnOpenGL：Fragment shader](https://learnopengl.com/Getting-started/Hello-Triangle)、[glCreateShader](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCreateShader.xhtml)、[glGetShaderiv](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetShader.xhtml)、[glGetShaderInfoLog](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetShaderInfoLog.xhtml)、[GLSL 3.30 公式仕様：出力変数](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.3.30.pdf)。

## 実習：シェーダーをリンクして三角形を描く

### リンク前の確認

GLSL の入口は正確に `void main()` と書く（実装時の `void mian()` は修正済み）。入口の main がそろっているかもリンク時の検査対象となるため、個々のコンパイル成功と、プログラムのリンク成功はそれぞれ確認する。

頂点・フラグメントの両方で、`glGetShaderInfoLog()` の第 2 引数を固定の 512 から、確保した文字列に対応する `infoLogLength` に変更する。

また、フラグメントシェーダーの作成が 0 で失敗した分岐では、`glfwTerminate()` の前に、作成済みの `glDeleteShader(vertexShader)` も呼ぶ。

### なぜプログラムにリンクするか

頂点シェーダーは位置を計算する処理、フラグメントシェーダーは色を計算する処理。リンクは、これらを描画で使う一組の実行用プログラムに構築する工程。必要な入口や、ステージ間の入力・出力の対応なども検査する。

```text
shaderProgram
    ├─ vertexShader   ：位置の計算
    └─ fragmentShader ：色の計算
```

プログラムは処理の内容、VAO は頂点データの読み方と VBO への参照、VBO は実際の頂点データを保持する。それぞれを準備し、描画時に使うものを選ぶ。

### 1. プログラムを作る

フラグメントシェーダーのコンパイル結果確認の後、`glfwGetFramebufferSize()` より前に追加する。この位置では VBO と VAO はまだ作っていない。

```cpp
GLuint shaderProgram = glCreateProgram();
if (shaderProgram == 0) {
    std::cerr << "Failed to create shader program\n";
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glfwTerminate();
    return -1;
}
```

`glCreateProgram()` は空のプログラムオブジェクトを作り、ID を直接返す。まだシェーダーは接続されていない。

### 2. シェーダーを接続してリンクする

```cpp
glAttachShader(shaderProgram, vertexShader);
glAttachShader(shaderProgram, fragmentShader);
glLinkProgram(shaderProgram);
```

`glAttachShader(program, shader)` の第 1 引数は接続先のプログラム、第 2 引数は含めたいシェーダーの ID。二つのシェーダーを同じプログラムへ接続し、`glLinkProgram()` で実行用の結果を構築する。

ここまででプログラムが構築されても、後続の描画で使うものを選ぶ `glUseProgram()` は別に必要。

### 3. リンクの成功・失敗を確認する

```cpp
GLint linked = GL_FALSE;
glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);

if (linked != GL_TRUE) {
    GLint programLogLength = 0;
    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &programLogLength);
    std::string programLog(programLogLength, '\0');
    glGetProgramInfoLog(shaderProgram, programLogLength, nullptr,
                        programLog.data());
    std::cerr << "Shader program link failed:\n" << programLog << '\n';
    glDeleteProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glfwTerminate();
    return -1;
}

glDeleteShader(vertexShader);
glDeleteShader(fragmentShader);
std::cout << "Shader program link succeeded\n";
```

コンパイル結果と同じ考え方で、プログラムのリンク結果を取得する。

| 確認するもの | 成功・失敗の取得 | 診断ログの取得 |
| --- | --- | --- |
| 個々のシェーダーのコンパイル | `glGetShaderiv(..., GL_COMPILE_STATUS, ...)` | `glGetShaderInfoLog()` |
| プログラムのリンク | `glGetProgramiv(..., GL_LINK_STATUS, ...)` | `glGetProgramInfoLog()` |

リンク成功後はプログラムが実行用の結果を保持するため、元のシェーダーオブジェクトには削除を要求できる。接続中の実際の削除は遅延するが、描画にはリンク済みプログラムを使える。

正常経路の `glDeleteShader()` の二行は、ループ後からこのリンク成功後へ移す。終了時には、下のプログラム・VAO・VBO の後片付けを使う。

### 4. ループ内でプログラムと VAO を選び、描画する

`while` の中、既存の `glClear(GL_COLOR_BUFFER_BIT)` の後、`glfwSwapBuffers(window)` より前に、次の三行を追加する。

```cpp
glUseProgram(shaderProgram);
glBindVertexArray(vao);
glDrawArrays(GL_TRIANGLES, 0, 3);
```

| 呼び出し | 描画時の役割 |
| --- | --- |
| `glUseProgram(shaderProgram)` | 頂点や色をどう計算するか、使うプログラムを選ぶ |
| `glBindVertexArray(vao)` | どの VBO をどの形式で読むか、使う頂点入力設定を選ぶ |
| `glDrawArrays(GL_TRIANGLES, 0, 3)` | 指定した三頂点を使って、三角形の描画を実行する |

`glDrawArrays()` の引数：

- `GL_TRIANGLES`：三つの頂点ごとに一つの三角形を組み立てる。
- `0`：先頭の頂点番号。属性番号 0 とは別の番号。
- `3`：使用する頂点の数。float の個数ではない。今回は各頂点に三つの float があり、三頂点で合計九つの float を読む。

プログラムと VAO の選択は状態として残るので、対象が変わらなければ、毎フレームの再選択は必須ではない。今回は描画の直前に使う対象を明示する。VBO の参照と読み方は VAO の属性に記録済みなので、ここで VBO の再バインドや属性の再設定を行う必要はない。

#### 毎フレームの VAO のバインドは必須か

現在のように一つの VAO だけを使い、途中で別の VAO に切り替えなければ、ループ内の `glBindVertexArray(vao)` は省略できる。初期化時に選んだ VAO が、その後も現在の VAO として保持される。

同じコンテキストでは、別の VAO をバインドする、`glBindVertexArray(0)` で解除する、バインド中の VAO を削除するなどして選択を変えるまで、その状態が続く。フレームの区切りや `glClear()`、`glfwSwapBuffers()` によって VAO の選択がリセットされることはない。

プログラムも一つで変更しない場合は、例えば次のように、使うものをループ前に選べる。VAO は初期化時から選択されたままなら、その選択をそのまま使ってもよい。

```cpp
glUseProgram(shaderProgram);
glBindVertexArray(vao);

while (!glfwWindowShouldClose(window)) {
    processInput(window);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glfwSwapBuffers(window);
    glfwPollEvents();
}
```

前の例でループ内に置いた目的は、「この描画で使う VAO」を描画命令の直前に明示するため。複数の図形へ進むと、その場所で VAO を切り替える形にもなる。

例えば、それぞれ三頂点を読む設定を持つ `vaoA` と `vaoB` を作成済みで、共通のシェーダープログラムを使用している場合、ループ内の描画部分を次のようにする。

```cpp
glBindVertexArray(vaoA);
glDrawArrays(GL_TRIANGLES, 0, 3);

glBindVertexArray(vaoB);
glDrawArrays(GL_TRIANGLES, 0, 3);
```

B を描いた後は B が選ばれたままで、次のフレームもその状態から始まる。そのため、A を描く前には A を選び直す。毎フレームの再バインドそのものではなく、「描画時に使いたい VAO が選ばれていること」が必要条件になる。

参考：[OpenGL 公式：glBindVertexArray](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindVertexArray.xhtml)。

一フレームの流れは、入力処理 → バッファの消去 → プログラム・VAO の選択 → 三角形の描画 → バッファ交換 → イベント処理、となる。

例えば複数の図形が同じシェーダーの入力形式に対応していれば、一つのプログラムを使ったまま VAO を切り替えて描ける。頂点の読み出し設定と、計算する処理を別々に再利用できる。

### 5. ループ後の後片付けを整理する

ループ後の処理は次の並びにする。`glDeleteShader()` の二行は上のリンク成功後へ移動済み。

```cpp
glUseProgram(0);
glDeleteProgram(shaderProgram);
glDeleteVertexArrays(1, &vao);
glDeleteBuffers(1, &vbo);
glfwTerminate();
```

`glUseProgram(0)` で現在のプログラムを解除し、`glDeleteProgram()` でプログラムを片付ける。すべての OpenGL オブジェクトの後片付けは、現在のコンテキストを終了する `glfwTerminate()` より前に行う。

### 今回の確認

- [x] フラグメントシェーダーの入口の綴りと、両シェーダーのログ取得容量を修正する。
- [x] プログラム作成・シェーダー接続・リンク・リンク結果確認を追加する。
- [x] リンク失敗時の `glfwTerminate()` の直後に `return -1;` を追加し、後続の処理へ進まないようにする。
- [x] 正常経路のシェーダー削除を、リンク成功後へ移す。
- [x] 描画で使うプログラム・VAO を選び、三頂点を描画する。VAO の選択は初期化時から保持している。
- [x] 終了時にプログラム・VAO・VBO を片付ける。
- [x] リンク済みプログラムを使って描画できることを、学習者の表示報告で確認する。
- [x] 背景の上にオレンジ色の三角形が表示される（2026-10-25、学習者から報告）。

2026-10-25：コードの追加と修正を確認し、学習者から三角形の表示報告を受けた。この実習は完了。

参考：[glCreateProgram](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCreateProgram.xhtml)、[glAttachShader](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glAttachShader.xhtml)、[glLinkProgram](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glLinkProgram.xhtml)、[glGetProgramiv](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetProgram.xhtml)、[glGetProgramInfoLog](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetProgramInfoLog.xhtml)、[glUseProgram](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glUseProgram.xhtml)、[glDrawArrays](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawArrays.xhtml)、[glDeleteProgram](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDeleteProgram.xhtml)。


---

[目次](../README.md) · [前の章](06_vertex_shader.md) · [次の章](08_ebo_and_indexed_drawing.md) · [早見表](reference.md)
