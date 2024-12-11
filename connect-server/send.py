import os
import requests
import json
import sys
from dotenv import load_dotenv

url = 'http://localhost:8080/answer'  # サーバーのURL

load_dotenv()
PROCON_TOKEN = os.getenv('PROCON_TOKEN')

# JSONファイルを読み込む
with open(sys.argv[1], 'r') as json_file:
    data = json.load(json_file)

headers = {'Content-Type': 'application/json', 'Procon-Token': PROCON_TOKEN}

# データを送信
response = requests.post(url, data=json.dumps(data), headers=headers)

print(response.json())