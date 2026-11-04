# 第2章：描画ループと viewport

[目次](../README.md) · [前の章](01_environment_and_window.md) · [次の章](03_pipeline_and_coordinates.md) · [早見表](reference.md)

## 実習：背景色を表示する

新しく使うのは `glClearColor`、`glClear`、`glfwSwapBuffers` の三つ。

**消去色（clear color）**は、カラーバッファをクリアするときに塗る色。絵を描く前に、キャンバスを一色で塗り直すときの色に相当する。ここでの「消去」は、以前の描画内容を指定した色で置き換え、次の描画の土台を用意すること。

`glClearColor()` で「何色で塗り直すか」を設定し、`glClear(GL_COLOR_BUFFER_BIT)` で実際に塗り直す。今の課題では、その後に図形を描いていないため、この色がそのまま背景として見える。

GLAD の初期化と失敗チェックの後に消去色を設定し、既存のループに消去とバッファ交換を追加する。

```cpp
glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

while (!glfwWindowShouldClose(window)) {
    processInput(window);

    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);
    glfwPollEvents();
}
```

既存のループの後にある `glfwTerminate()` は、このループが終わってから実行する。

| 関数 | 今回の意味 |
| --- | --- |
| `glClearColor(r, g, b, a)` | カラーバッファを消去するときの色を設定。RGBA をそれぞれ 0～1 で指定し、今回はアルファを `1.0f` にする |
| `glClear(GL_COLOR_BUFFER_BIT)` | 現在の消去色で、描画先のカラーバッファを初期化する |
| `glfwSwapBuffers(window)` | 裏のバッファで用意した画像を、表の表示用バッファとして表示できるよう交換する |

`glClearColor()` はコンテキストの状態を変更する関数。一度指定した色は、変更するまで記憶されるので、固定色ならループの外で設定できる。実際に色データを書き込む `glClear()` は、今回は毎フレーム呼び出す。

GLFW のウィンドウは標準でダブルバッファリングを使う。通常は裏のバッファへ描画し、描画後に `glfwSwapBuffers()` で表示用の表のバッファと交換する。

一回のループが「1 フレーム」に相当する。今回の順序は、入力を読む → カラーバッファを消去 → バッファを交換 → イベントを処理、となる。

### 自分で書いて確認すること

- [ ] 上の処理を追加し、青緑色の背景が表示される。
- [ ] 消去色を `(0.8f, 0.2f, 0.2f, 1.0f)` に変更し、再ビルド・実行すると赤系の背景になる。
- [ ] Esc キー、または閉じるボタンで正常に終了する。

色を変えたときに確認するのは、「消去色という状態を変えると、その状態を使う `glClear()` の結果も変わる」という関係。

参考：[LearnOpenGL：背景色とダブルバッファリング](https://learnopengl.com/Getting-started/Hello-Window)、[GLFW：バッファ交換](https://www.glfw.org/docs/3.4/window_guide.html#buffer_swap)。

## 実習：glViewport とサイズ変更への対応

### glViewport の役割

`glViewport(x, y, width, height)` は、描く図形の座標を、フレームバッファ上のどの長方形に対応付けるかを決める。設定はコンテキストの状態として記憶される。

| 引数 | 意味 |
| --- | --- |
| `x`, `y` | 視口の左下の位置。単位はピクセル |
| `width` | 視口の幅。単位はピクセル |
| `height` | 視口の高さ。単位はピクセル |

今回は `x = 0`、`y = 0` とし、幅・高さを実際のフレームバッファのサイズに合わせ、全体を視口にする。

ウィンドウのサイズはスクリーン座標、フレームバッファのサイズはピクセルで表す。Retina などの画面では両者が一致しない場合があるため、`glfwGetFramebufferSize()` でピクセル数を取得する。描画先を別の倍率のディスプレイへ移した際にも、フレームバッファのサイズが変わる場合がある。

### 1. サイズ変更時に呼ばれる関数を用意する

`main()` より前に定義する。

```cpp
static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    std::cout << "Framebuffer: " << width << " x " << height << '\n';
}
```

この関数は自分で定義するもので、GLFW の組み込み関数ではない。登録しておくと、フレームバッファのサイズが変わったときに GLFW が呼び、新しい幅・高さを渡す。このように呼んでもらう関数をコールバックという。

引数は、GLFW がこの種類のコールバックの仕様として決めている。

| 引数 | GLFW から渡される情報 |
| --- | --- |
| `GLFWwindow* window` | フレームバッファのサイズが変わったウィンドウのハンドル |
| `int width` | 新しいフレームバッファの幅（ピクセル） |
| `int height` | 新しいフレームバッファの高さ（ピクセル） |

複数のウィンドウに同じ関数を登録した場合も、`window` でどのウィンドウの変更かを区別できる。例えば B の変更なら、GLFW は概念的に `framebufferSizeCallback(windowB, newWidth, newHeight)` のように呼び出す。

現在の関数では `window` を使用していないが、登録する関数の型を GLFW の仕様に合わせるため、最初の `GLFWwindow*` 引数は残す。引数の構成はコールバックの種類ごとに決まり、すべてのコールバックがこの 3 引数を取るわけではない。

`window` が渡されることと、そのウィンドウのコンテキストが現在のスレッドの使用対象になることは別。今回のプログラムではウィンドウが一つで、すでにそのコンテキストを使用対象にしている。

`processInput()` は自分でループから呼ぶのに対し、この関数は GLFW に登録して呼んでもらう。今回のコンソール出力は、呼び出されたことと新しいピクセル数を確認するために使う。

### 2. 初期サイズで設定し、コールバックを登録する

GLAD の初期化と失敗チェックが終わった後、`glClearColor()` より前に追加する。

```cpp
int framebufferWidth = 0;
int framebufferHeight = 0;
glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
glViewport(0, 0, framebufferWidth, framebufferHeight);
glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
```

最初にサイズを取得して視口を設定することで、サイズ変更のイベントが来る前から正しい視口を使える。コールバックの登録はここで一度行う。

登録時に渡すのは `framebufferSizeCallback` という関数そのもの。ここでは `()` を付けて呼び出さない。GLFW がサイズの変更を通知するときに呼ぶ。

今のループにある `glfwPollEvents()` でサイズ変更のイベントが処理されると、このコールバックが呼ばれ、その中の `glViewport()` が新しいサイズを設定する。

### 3. 確認すること

- [ ] ビルドして実行し、既存の背景色と Esc による終了が動く。
- [ ] ウィンドウの端をドラッグすると、コンソールに新しい幅・高さが表示される。
- [ ] 表示されたピクセル数とウィンドウ作成時の幅・高さが違う場合は、表示倍率との関係を観察する。

`glClear()` の消去範囲は `glViewport()` によって制限されない。そのため、背景色だけの現在のプログラムでは、視口を変えても背景は全体に表示される。今回はコールバックの出力で通知を確認し、視口が図形の表示に与える効果は、三角形を描く段階で確認する。

参考：[GLFW：フレームバッファのサイズとコールバック](https://www.glfw.org/docs/3.4/window_guide.html#window_fbsize)、[LearnOpenGL：視口](https://learnopengl.com/Getting-started/Hello-Window)。

### トラブル：図形が小さい・左下に寄る

2026-11-02 の書き直しでは、ウィンドウ作成に使った固定値 `640 × 480` を、初期の `glViewport()` にもそのまま渡していた。

`glfwCreateWindow()` の幅・高さはスクリーン座標で、`glViewport()` の幅・高さはフレームバッファのピクセル数。変数を `framebufferWidth` と名付けても、ウィンドウ作成時の指定値が実際のピクセル数になるわけではない。

Retina では、例えばウィンドウが `640 × 480` でもフレームバッファが `1280 × 960` になる場合がある。そのとき `glViewport(0, 0, 640, 480)` を指定すると、NDC 全体の対応先がフレームバッファの左下 1/4 の領域になる。この値は例であり、実際のサイズは取得して確認する。

GLAD 初期化後、描画ループに入る前に、固定値を渡していた初期の `glViewport()` を次の処理へ置き換える。

```cpp
int fbWidth = 0;
int fbHeight = 0;
glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
glViewport(0, 0, fbWidth, fbHeight);
std::cout << "Framebuffer: " << fbWidth << " x " << fbHeight << '\n';
```

`glfwGetFramebufferSize()` は、現在の実際の幅・高さを出力引数へ書き込む。取得したピクセル数で視口を設定し、出力した値で表示倍率による違いも確認する。

既存の `glfwSetFramebufferSizeCallback(window, framebufferSizeCallback)` は、サイズ変更時の対応として使う。登録自体が初期の視口設定を行うわけではないため、起動時には上の取得・設定も行う。サイズ変更で図形の位置や大きさが急に直る場合は、初期設定とコールバックの設定値の違いが手がかりになる。

さらに、頂点座標による図形自身の大きさも区別する。現在の頂点シェーダーは `gl_Position = vec4(aPos, 1.0)` なので、入力の x・y と NDC が同じ。四頂点の x・y は `−0.5 ～ 0.5`、表示範囲は `−1 ～ 1` なので、図形の幅・高さはそれぞれ視口の半分となる。

| フレームバッファを 1280 × 960 とした例 | 図形の幅 × 高さ | 位置 |
| --- | --- | --- |
| 視口が 640 × 480 のまま | 320 × 240 ピクセル | 左下の視口の中央 |
| 視口を 1280 × 960 に合わせる | 640 × 480 ピクセル | ウィンドウ内容全体の中央 |

視口を合わせた後、図形をさらに大きくしたければ、頂点の x・y の `±0.5` を `±0.8` にすると、幅・高さがそれぞれ視口の 80% になる。まずは視口を正しく設定し、それから頂点座標で大きさを調整する。

実際のフレームバッファサイズ、視口の修正後の表示結果は未確認。

参考：[GLFW：ウィンドウサイズとピクセル数の違い](https://www.glfw.org/docs/3.4/window_guide.html#window_fbsize)、[glViewport：NDC とピクセル座標の対応](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glViewport.xhtml)。座標の変換式は[第3章](03_pipeline_and_coordinates.md)を参照。


---

[目次](../README.md) · [前の章](01_environment_and_window.md) · [次の章](03_pipeline_and_coordinates.md) · [早見表](reference.md)
