import json
import os
import requests
from dotenv import load_dotenv

url = "http://localhost:8080/problem"

# 環境変数(.env)をロードする
load_dotenv()

# 環境変数からPROCON_TOKENを取り出す
PROCON_TOKEN = os.getenv('PROCON_TOKEN')

headers = {'Procon-Token': PROCON_TOKEN}

# リクエストを送信する
r = requests.get(url, headers=headers)

# output.jsonを出力する
with open('./problem.json', 'w') as f:
    json.dump(r.json(), f, ensure_ascii=False)