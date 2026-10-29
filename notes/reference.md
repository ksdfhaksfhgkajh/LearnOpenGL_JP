# 関数・GLSL の早見表

[目次](../README.md)

## GLFW 関数の早見表

これまでコードで使用した関数と、今回の実習で追加する関数。

| 関数 | 役割・戻り値 | 使う場所 |
| --- | --- | --- |
| `glfwInit()` | GLFW を初期化。成功は `GLFW_TRUE`、失敗は `GLFW_FALSE` | 起動時 |
| `glfwWindowHint(hint, value)` | これから作るウィンドウやコンテキストの条件を指定。戻り値なし | 初期化後、ウィンドウ作成前 |
| `glfwCreateWindow(width, height, title, monitor, share)` | ウィンドウと関連するコンテキストを作成。成功は `GLFWwindow*`、失敗は `nullptr` | 条件を設定した後 |
| `glfwMakeContextCurrent(window)` | 現在のスレッドで使用するコンテキストを選択。戻り値なし | GLAD の初期化前 |
| `glfwGetProcAddress(name)` | 現在のコンテキストの OpenGL 関数のアドレスを取得。失敗はヌルポインタ | GLAD に検索関数として渡す |
| `glfwWindowShouldClose(window)` | 終了要求のフラグを取得。立っていれば真 | ループ条件 |
| `glfwGetKey(window, key)` | 最後に報告されたキーの状態を取得。`GLFW_PRESS` または `GLFW_RELEASE` | 入力処理 |
| `glfwSetWindowShouldClose(window, value)` | 終了要求のフラグを設定。戻り値なし。ウィンドウの破棄自体は行わない | Esc キーなどで終了を要求するとき |
| `glfwPollEvents()` | 届いている OS のイベントを処理し、入力状態などを更新。戻り値なし | 毎フレーム |
| `glfwSwapBuffers(window)` | 表示用と描画用のカラーバッファを交換。戻り値なし | 毎フレーム、描画の後 |
| `glfwGetFramebufferSize(window, &width, &height)` | 描画先の幅・高さをピクセル単位で取得。出力引数に値を書き込む。戻り値なし | 初期の視口を設定するとき |
| `glfwSetFramebufferSizeCallback(window, callback)` | フレームバッファのサイズが変化したときの関数を登録。戻り値は以前のコールバック（未登録なら `nullptr`） | 今回は GLAD 初期化後、ループに入る前 |
| `glfwTerminate()` | 残っているウィンドウを破棄し、GLFW が管理するリソースを解放。戻り値なし | 正常終了時、初期化成功後のエラー終了時 |

`processInput()` は、自分で定義した入力処理の関数。内部で GLFW の `glfwGetKey()` や `glfwSetWindowShouldClose()` を呼び出している。

`glfwPollEvents()` は OS から届いたイベントを処理する関数で、ハードウェア割り込みを直接処理する関数ではない。

参考：[GLFW 3.4 入門](https://www.glfw.org/docs/3.4/quick_guide.html)、[キー入力](https://www.glfw.org/docs/3.4/input_guide.html#input_key)、[コンテキストと関数の読み込み](https://www.glfw.org/docs/3.4/context_guide.html)。

## OpenGL 関数の早見表

`glClear` などは、GLAD で読み込んで呼び出す場合も、OpenGL の描画 API としてここに記録する。以下の関数を呼ぶ際は、現在のコンテキストと GLAD の初期化が必要。

| 関数 | 役割・戻り値 | 使う場所 |
| --- | --- | --- |
| `glClearColor(r, g, b, a)` | カラーバッファの消去色を RGBA で設定。戻り値なし | 今回は GLAD 初期化後、ループ前に一度。色を変更するときにも呼ぶ |
| `glClear(mask)` | 指定したバッファを消去。今回は `GL_COLOR_BUFFER_BIT` で色を初期化。戻り値なし | 毎フレーム、図形の描画前 |
| `glViewport(x, y, width, height)` | 図形の座標を対応付ける視口をピクセル単位で設定。戻り値なし | 初期のピクセル数取得後と、フレームバッファのサイズ変更時 |
| `glGenBuffers(count, ids)` | 未使用のバッファ名（整数の ID）を取得し、出力先へ保存。VBO・EBO で共通。戻り値なし | VBO・EBO の初期準備 |
| `glBindBuffer(target, id)` | 指定した対象にバッファを結び付ける。初回にオブジェクトを作成。今回は `GL_ARRAY_BUFFER` で頂点用の操作先を選び、`GL_ELEMENT_ARRAY_BUFFER` で現在の VAO に EBO の参照を記録する。戻り値なし | データ転送前、頂点入力・インデックス入力の準備 |
| `glBufferData(target, size, data, usage)` | 現在その対象に結び付いたバッファに格納領域を確保し、指定データをコピー。`size` はバイト数、`usage` は使い方のヒント。戻り値なし | 今回は VBO・EBO の初期化時にそれぞれ一度 |
| `glDeleteBuffers(count, ids)` | 指定したバッファを削除。VBO・EBO で共通。戻り値なし | ループ終了後、今回は VAO の削除後、`glfwTerminate()` より前 |
| `glGenVertexArrays(count, ids)` | 未使用の VAO の ID を取得し、出力先へ保存。戻り値なし | 頂点入力の初期準備 |
| `glBindVertexArray(id)` | 設定・描画で使う VAO を選択。属性設定と、その VAO に保存した EBO のバインドが選ばれる。戻り値なし | 頂点属性・EBO の設定前、描画対象を切り替えるとき |
| `glVertexAttribPointer(index, size, type, normalized, stride, pointer)` | 属性の読み方と、その時点の `GL_ARRAY_BUFFER` の VBO 参照を、現在の VAO に記録。戻り値なし | VAO と VBO を選択した後 |
| `glEnableVertexAttribArray(index)` | 指定属性を、配列から読み出すよう有効化。現在の VAO に記録される。戻り値なし | 属性の初期設定時 |
| `glDeleteVertexArrays(count, ids)` | VAO を削除。VBO のデータを削除する関数とは別。戻り値なし | ループ後、今回は VBO の削除より前 |
| `glCreateShader(type)` | `GL_VERTEX_SHADER` は頂点用、`GL_FRAGMENT_SHADER` はフラグメント用のオブジェクトを作成。戻り値は `GLuint` の ID、作成失敗時は 0 | GLAD 初期化後、各シェーダーのソース登録前 |
| `glShaderSource(shader, count, strings, lengths)` | `count` 個の文字列をシェーダーへコピー。`strings` は文字列ポインターの配列、`lengths` が `nullptr` なら各文字列をヌル終端として扱う。戻り値なし | 作成したシェーダーのコンパイル前 |
| `glCompileShader(shader)` | 登録済みの GLSL ソースをコンパイルし、結果をシェーダーオブジェクトに保持。戻り値なし | ソース登録後、初期化時 |
| `glGetShaderiv(shader, pname, &value)` | `GL_COMPILE_STATUS` で成功・失敗、`GL_INFO_LOG_LENGTH` で終端を含むログの必要容量を取得。ログが空なら容量は 0 の場合もある。値は整数の出力引数へ書き込む。戻り値なし | コンパイル後の結果確認・ログ領域の確保 |
| `glGetShaderInfoLog(shader, capacity, &length, buffer)` | 診断ログを文字配列へ取得。`capacity` は実際に確保した保存先の容量以下にする。長さを使わなければ第 3 引数は `nullptr`。戻り値なし | 失敗の原因や警告を確認するとき |
| `glDeleteShader(shader)` | シェーダーの削除を要求。引数は ID 自体。プログラムに接続中は実際の削除が遅延する。戻り値なし | 今回はリンク成功後、または失敗時。正常経路ではループ後からリンク成功後へ移す |
| `glCreateProgram()` | 空のプログラムオブジェクトを作成。戻り値は `GLuint` の ID、作成失敗時は 0 | 今回は両シェーダーのコンパイル成功後 |
| `glAttachShader(program, shader)` | リンクに含めるシェーダーをプログラムへ接続。戻り値なし | プログラム作成後、リンク前 |
| `glLinkProgram(program)` | 接続済みのシェーダーから、描画で使う実行用のプログラムを構築。戻り値なし | 必要なシェーダーを接続した後、初期化時 |
| `glGetProgramiv(program, pname, &value)` | `GL_LINK_STATUS` でリンクの成功・失敗、`GL_INFO_LOG_LENGTH` でログの必要容量を取得。値は整数の出力引数へ書き込む。戻り値なし | リンク後の結果確認 |
| `glGetProgramInfoLog(program, capacity, &length, buffer)` | プログラムの診断ログを文字配列へ取得。容量を実際の保存先に合わせる。戻り値なし | リンク失敗の原因を確認するとき |
| `glUseProgram(program)` | 後続の描画に使うシェーダープログラムを選ぶ。0 を渡すと現在のプログラムを解除。戻り値なし | リンク成功後、描画前。今回はループ内で選び、終了時に解除 |
| `glDrawArrays(mode, first, count)` | 現在のプログラムと VAO の頂点入力を使って描画。`mode` は組み立てるプリミティブ、`first` は先頭の頂点番号、`count` は頂点数。戻り値なし | 毎フレーム、消去後・バッファ交換前 |
| `glDrawElements(mode, count, type, offset)` | 現在の VAO の EBO から頂点番号を読み、その順番で描画。`count` は読むインデックスの個数、`type` はインデックスの整数型、`offset` は EBO 内のバイト位置をポインター型で指定。戻り値なし | EBO を使う描画。今回は消去後・バッファ交換前 |
| `glPolygonMode(face, mode)` | ポリゴンのラスタライズ方法を設定。Core Profile では `face` は `GL_FRONT_AND_BACK`。`GL_LINE` は辺を線表示、`GL_FILL` は内部を塗りつぶす（初期値）。戻り値なし | 初期表示の設定、キー入力などで表示方法を切り替えるとき |
| `glDeleteProgram(program)` | プログラムの削除を要求。使用中は削除が遅延する。戻り値なし | リンク失敗時、またはループ後。今回は `glUseProgram(0)` の後、`glfwTerminate()` より前 |

`glClearColor()` と `glViewport()` は設定を記憶させる関数。`glClear()` は、現在の設定を使って実際にバッファを書き換える関数。

参考：[LearnOpenGL：Hello Window](https://learnopengl.com/Getting-started/Hello-Window)、[Hello Triangle](https://learnopengl.com/Getting-started/Hello-Triangle)。

## GLAD 関数の早見表

このプロジェクトでは GLAD 1 の初期化 API を使う。

| 関数 | 役割・戻り値 | 使う場所 |
| --- | --- | --- |
| `gladLoadGLLoader(loader)` | 渡した検索関数を使い、OpenGL 関数のアドレスを読み込む。成功は `0` 以外、失敗は `0` | `glfwMakeContextCurrent()` の後、OpenGL 関数を使う前に一度 |

`loader` には `reinterpret_cast<GLADloadproc>(glfwGetProcAddress)` を渡す。`GLADloadproc` は関数ポインタの型名で、関数ではない。

参考：[GLFW：GLAD による関数の読み込み](https://www.glfw.org/docs/3.4/context_guide.html#context_glext_auto)。

## GLSL の早見表

| 記号・型 | 役割と今回の使い方 |
| --- | --- |
| `#version 330 core` | GLSL 3.30 Core を使う宣言 |
| `vec2`・`vec3`・`vec4` | それぞれ二・三・四成分の float を持つ型。位置や色など、使う変数によって意味を与える |
| `layout (location = N) in` | 頂点シェーダーの入力を、VAO の属性番号 N に対応させる |
| `in` | そのシェーダーステージへの入力。今回は頂点側で位置・色を受け取り、フラグメント側で補間された色を受け取る |
| `out` | そのシェーダーステージからの出力。今回は頂点側で色を渡し、フラグメント側で描画用の色を出す |
| `gl_Position` | 頂点シェーダーの組み込み出力変数。クリップ座標の `vec4` を代入する |
| `vec4(aPos, 1.0)` | 三成分に四つ目の値を補って vec4 を構築。gl_Position に使う場合は x・y・z・w |
| `vec4(vertexColor, 1.0)` | RGB の三成分にアルファを補って RGBA の出力を構築 |

GLSL 3.30 の今回の書き方では、頂点シェーダーの out とフラグメントシェーダーの in は、名前・型・補間に関する指定を対応させる。頂点属性の location 番号による接続と、ステージ間の名前による接続を区別する。

参考：[LearnOpenGL：Shaders](https://learnopengl.com/Getting-started/Shaders)、[GLSL 3.30 公式仕様](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.3.30.pdf)。


---

[目次](../README.md)
