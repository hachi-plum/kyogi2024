# procon35 server
## 簡易サーバーを動かす方法
以下のような設定ファイルを用意し, server コマンドに -c で指定して実行すると 10 秒後に試合が開始されます。
```
{
  "teams": [
    "token1",
    "token2",
    "token3"
  ],
  "duration": 300,
  "problem": {
    "board": {
      "width": 6,
      "height": 4,
      "start": [
        "220103",
        "213033",
        "022103",
        "322033"
      ],
      "goal": [
        "000000",
        "111222",
        "222233",
        "333333"
      ]
    },
    "general": {
      "n": 2,
      "patterns": [
        {
          "p": 25,
          "width": 4,
          "height": 2,
          "cells": [
            "0111",
            "1001"
          ]
        },
        {
          "p": 26,
          "width": 2,
          "height": 2,
          "cells": [
            "10",
            "01"
          ]
        }
      ]
    }
  }
}
```
`teams` にはチームのトークン情報を記述します。
`duration` には試合時間を秒で記述します。
`problem` には問題の情報を記述します。problemの形式については募集要項を参照してください。
プログラムのlistenするポートはデフォルトでは 3000 番ですが -l :8000 のように実行時の引数に渡すと使用するポートを変更することができます。 または環境変数 PORT によっても指定できます。その際には PORT=8000 のように指定してください。

```
procon-server_darwin_arm -c input.json -l :8080 -start 10s
```

起動してから試合開始までの時間はデフォルトで 10 秒になっていますが -start 1s のように引数で渡すことで変更することができます。

## 問題情報取得
`/problem` にtokenをつけてGETリクエストを送ることで試合情報が取得できます。
試合時間内であれば設定ファイルで記述したproblemが返却されます。

```
curl -v localhost:8080/problem -H "Procon-Token: token1"
```

## 回答提出方法
`/answer` にjsonをPOSTすることで回答を送信できます。
回答の形式については募集要項を参照してください。
```json
{
  "n":3,
  "ops":[
    {
      "p":4,
      "x":1,
      "y":1,
      "s":2
    },
    {
      "p":11,
      "x":3,
      "y":-12,
      "s":3
    },
    {
      "p":25,
      "x":3,
      "y":3,
      "s":0
    }
  ]
}
```
回答が受理されるとrevisionという回答を受け付けた番号が返却されます。
```
curl -X POST -H "Content-Type: application/json" -H "Procon-Token: token1" localhost:8080/answer -d @solution.json
```
