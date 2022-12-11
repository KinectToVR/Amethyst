import json
from pathlib import Path

files = []
for path in Path('').rglob('*.cs'):
    files.append(path)

for path in Path('../K2CrashHandler').rglob('*.cs'):
    files.append(path)

print(files)

# Opening JSON file
f = open('Assets/Strings/fr.json', 'r', encoding="utf8")

# returns JSON object as
# a dictionary
data = json.load(f)

# Iterating through the json
notFound = []
for key in data:
    notFound.append(key)
    for file in files:
        with open(file, 'r') as f:
            for index, line in enumerate(f):
                if key in notFound and (("/DevicesPage/Devices/Manager/Labels/" in key or "/SharedStrings/Joints/" in key) or key in line or key.removeprefix("/CrashHandler/") in line):
                    notFound.remove(key)

notFound = list(dict.fromkeys(notFound))
for key in notFound:
    print(f'"{key}" not found in any file!')
    data.pop(key)

# Closing file
f.close()

f = open('Assets/Strings/fr.json', 'r+', encoding="utf8")
f.truncate(0)

json.dump(data, f, indent=2, ensure_ascii=False)
