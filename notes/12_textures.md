# 第12章：テクスチャと UV 座標

[目次](../README.md) · [前の章](11_uniforms.md) · [早見表](reference.md)

## 実習：UV 座標を色として表示する

### なぜテクスチャを使うか

頂点色の補間でグラデーションを描けるようになった。写真や木目、文字などの細かい模様には、画像を参照して色を決める方法を使える。今回は、そのための二次元テクスチャを学ぶ。

テクスチャを図形へ対応付けるために、各頂点へ UV 座標を追加する。位置 `(x, y, z)` は図形をどこに置くか、UV `(u, v)` は画像のどの位置を参照するかを表す。

この実習では u を横方向、v を縦方向とし、画像全体を 0～1 で表す。これは画像のピクセル数に依存しない基準。UV に範囲外の値も指定でき、その扱いは後で学ぶテクスチャの繰り返し設定などで決まる。

現在の四頂点と、UV の対応は次のようにする。右の色は、UV を `(u, v, 0)` の RGB として表示し、brightness が 1 の場合。

| 頂点 | 位置 | UV | UV を色にした結果 |
| --- | --- | --- | --- |
| 0 | 右上 | `(1, 1)` | 黄 |
| 1 | 右下 | `(1, 0)` | 赤 |
| 2 | 左下 | `(0, 0)` | 黒 |
| 3 | 左上 | `(0, 1)` | 緑 |

### 1. 一頂点を八成分にする

`src/main.cpp` の `quadVertices` を、位置 3 ＋頂点色 3 ＋ UV 2 の配列にする。

```cpp
constexpr GLfloat quadVertices[] = {
    // 位置              // 頂点色         // UV
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f
};
```

既存の `glBufferData()` は `sizeof(quadVertices)` を使っているので、この八成分の配列全体が VBO にコピーされる。EBO の一つのインデックスは、位置・頂点色・UV をまとめて一頂点として選ぶ。

### 2. VAO に属性 2 を設定する

既存の頂点属性設定の場所で、次の六つの呼び出しを使う。この位置では VAO と VBO が選択されている。

```cpp
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                      8 * sizeof(GLfloat), nullptr);
glEnableVertexAttribArray(0);

glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                      8 * sizeof(GLfloat),
                      reinterpret_cast<const void*>(3 * sizeof(GLfloat)));
glEnableVertexAttribArray(1);

glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                      8 * sizeof(GLfloat),
                      reinterpret_cast<const void*>(6 * sizeof(GLfloat)));
glEnableVertexAttribArray(2);
```

| 属性番号 | 内容 | 成分数 | stride | offset |
| --- | --- | --- | --- | --- |
| 0 | 位置 | 3 | `8 * sizeof(GLfloat)` | 0 |
| 1 | 頂点色 | 3 | `8 * sizeof(GLfloat)` | `3 * sizeof(GLfloat)` |
| 2 | UV | 2 | `8 * sizeof(GLfloat)` | `6 * sizeof(GLfloat)` |

属性 2 の最初の引数は属性番号、次の引数は成分数。今回はどちらも 2 だが、意味は別。UV の開始位置は位置と色の六成分の後なので offset は六つの float。

一頂点の全体が八成分になったため、属性 0・1 の stride も八成分へ変更する。この環境では一頂点は 32 バイト、UV の offset は 24 バイト。

### 3. 頂点シェーダーで UV を受け渡す

`shaders/vertex.glsl` を次の構成にする。

```glsl
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 vertexColor;
out vec2 texCoord;

void main() {
    gl_Position = vec4(aPos, 1.0);
    vertexColor = aColor;
    texCoord = aTexCoord;
}
```

`aTexCoord` は属性 2 から受け取る入力。`texCoord` はフラグメント側へ渡す出力。それぞれ二成分の `vec2` を使う。

頂点で指定した UV も、色と同じようにラスタライズ時に補間される。今回の `gl_Position.w` は全頂点で 1 なので、図形の中央付近には `(0.5, 0.5)` に近い UV が渡る。

### 4. フラグメントシェーダーで UV を可視化する

`shaders/fragment.glsl` を次の構成にする。

```glsl
#version 330 core

out vec4 fragColor;
in vec3 vertexColor;
in vec2 texCoord;
uniform float brightness;

void main() {
    vec3 uvColor = vec3(texCoord, 0.0);
    fragColor = vec4(uvColor * brightness, 1.0);
}
```

`vec3(texCoord, 0.0)` は `(u, v, 0)` の三成分を作る。赤成分が u、緑成分が v なので、右ほど赤く、上ほど緑が強くなる。UV の対応を色で確かめる実習。

現在の `brightness` の更新により、UV から作った色全体も明暗を繰り返す。C++ 側は既存の uniform 更新を使える。

GLSL の編集後は CMake を再読み込みし、ビルド先の `shaders/` のコピーも更新する。

### 今回の確認

- [ ] 各頂点に UV 二成分を追加する。
- [ ] 属性 0・1・2 すべての stride を八成分にする。
- [ ] 属性 2 を有効にし、頂点シェーダーの location 2 と対応させる。
- [ ] `out vec2 texCoord` と `in vec2 texCoord` の名前・型を一致させる。
- [ ] 右ほど赤く、上ほど緑が強いグラデーションと、既存の明るさの変化を確認する。

2026-11-04：第11章の終了順序の修正を受け、この実習を開始。UV の追加と表示結果は未確認。

続いて、画像データをテクスチャへ保存し、補間された UV を使って画像の色を取得する。画像の色を取得する処理をサンプリングと呼ぶ。

参考：[LearnOpenGL：Textures](https://learnopengl.com/Getting-started/Textures)、[glVertexAttribPointer](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml)。

---

[目次](../README.md) · [前の章](11_uniforms.md) · [早見表](reference.md)
