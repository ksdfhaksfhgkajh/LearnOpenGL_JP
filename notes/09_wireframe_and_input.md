# 第9章：ワイヤーフレームとキー入力

[目次](../README.md) · [前の章](08_ebo_and_indexed_drawing.md) · [次の章](10_glsl_in_out_and_colors.md) · [早見表](reference.md)

## 実習：線表示と塗りつぶし表示をキーで切り替える

### 何を観察するか

四角形を線だけで表示すると、外枠に加え、EBO で共有した対角線 0–2 が見える。各三角形の辺を描くため、四角形が二つの三角形からできていることを確認できる。このような表示をワイヤーフレーム表示と呼ぶ。

### glPolygonMode の役割

```cpp
glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
```

| 引数・値 | 意味 |
| --- | --- |
| `GL_FRONT_AND_BACK` | 表向き・裏向き両方のポリゴンに適用。今回の Core Profile ではこの値を使う |
| `GL_LINE` | ポリゴンの辺を線としてラスタライズする |
| `GL_FILL` | ポリゴンの内部を塗りつぶす。初期状態のモード |

この関数は戻り値を持たず、コンテキストのラスタライズ設定を変更する。その後のポリゴン描画に適用され、別のモードへ変更するまで状態が残る。VAO や個々のシェーダープログラムに保存する設定ではない。

```text
EBO の番号で三角形を組み立てる
    → ラスタライズ時に GL_FILL / GL_LINE の設定を使う
    → 生成されたフラグメントの色をフラグメントシェーダーで計算する
```

`glDrawElements(GL_TRIANGLES, ...)` は引き続き三角形を作る。`GL_LINE` にすると、それぞれの三角形の辺に沿ったフラグメントを生成する。線の色も、現在のフラグメントシェーダーが出すオレンジ色になる。

### 現在のコードと、実行するタイミング

既にループ前に `GL_LINE`、ループ後に `GL_FILL` を設定するコードがある。ループ後の行は、ウィンドウを閉じるなどして描画ループを抜けてから実行されるため、実行中の表示を切り替える操作にはならない。

今回は初期表示を塗りつぶしにし、実行中にキーで切り替える。ループ前の一行を次へ変更する。

```cpp
glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
```

初期値も `GL_FILL` だが、既にある `GL_LINE` の行を上記へ変更して、開始時の状態を明示する。

### processInput にキー操作を追加する

既存の `processInput()` の中で、Esc キーの if の後に次の二つの if を追加する。

```cpp
if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
```

| キー | 定数 | 設定するモード |
| --- | --- | --- |
| 1 | `GLFW_KEY_1` | `GL_LINE`：線表示 |
| 2 | `GLFW_KEY_2` | `GL_FILL`：塗りつぶし表示 |

`glfwGetKey()` の使い方は Esc キーで学んだものと同じ。現在の `processInput()` は毎フレームの描画前に呼ばれるので、そこで変更したモードがそのフレームの描画に適用される。

キーを押している間は、同じモードを繰り返し指定する。キーを離すと両方の if の本体を通らず、最後に選んだモードが残る。離した後も線表示や塗りつぶし表示が維持されることを観察する。

### 今回の確認

- [x] 線表示・塗りつぶし表示の設定コードを追加済みであることを確認する。
- [x] 初期表示を塗りつぶしにする（現在のコードでは OpenGL の既定の GL_FILL を使用）。
- [x] processInput に 1・2 キーの処理を追加する。
- [ ] 1 キーで外枠と対角線が表示される。
- [ ] 2 キーで内部が塗りつぶされる。
- [ ] キーを離しても最後に選んだモードが残る。

キー操作の追加をコード上で確認済み。モード切り替え時の表示の詳細は未報告。

参考：[OpenGL 公式：glPolygonMode](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml)、[GLFW 3.4：キー入力](https://www.glfw.org/docs/3.4/input_guide.html#input_key)。


---

[目次](../README.md) · [前の章](08_ebo_and_indexed_drawing.md) · [次の章](10_glsl_in_out_and_colors.md) · [早見表](reference.md)
