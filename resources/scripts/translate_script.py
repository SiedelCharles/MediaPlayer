import re
from openai import OpenAI

def process_srt(source_file, output_file1, output_file2):
    # print(source_file)
    pattern = re.compile(r'^\[\d{2}:\d{2}:\d{2}\.\d{3} --> \d{2}:\d{2}:\d{2}\.\d{3}\]\s*')
    with open(source_file, 'r', encoding='utf-8') as fin, \
         open(output_file1, 'a', encoding='utf-8') as fout1, \
         open(output_file2, 'a', encoding='utf-8') as fout2:
        for line in fin:
            match = pattern.match(line)
            processsed_line = pattern.sub("", line)
            fout1.write(f"{match.group().rstrip()}\n")
            fout2.write(processsed_line)

def translate(processed_text, translated_text):
    client = OpenAI(
        api_key="sk-06e36ac0d1af406bb1749744c48b943a",
        base_url="https://api.deepseek.com"
    )
    with open(processed_text, 'r', encoding='utf-8') as fin, \
         open(translated_text, 'a', encoding='utf-8') as fout:
            fin_context = fin.read()
            response = client.chat.completions.create(
                model = "deepseek-chat",
                messages = [
                    {
                        "role": "system",
                        "content": """ 你是一个翻译助手,负责将日语翻译成中文,要求保持原有的换行,也不要添加多余的换行"""
                    },
                    {
                        "role": "user",
                        "content": fin_context
                    }
                ],
            )
            if response.choices and response.choices[0].message.content is not None:
                translated_line = response.choices[0].message.content
                fout.write(translated_line + "\n")

def combination(timestamp_text, translated_text, output_text):
        with open(timestamp_text, 'r', encoding='utf-8') as fin1, \
             open(translated_text, 'r', encoding='utf-8') as fin2, \
             open(timestamp_text, 'a', encoding='utf-8') as fout:
            for line1, line2 in fin1, fin2:
                fout.write(f"{line1.strip()} {line2.strip()}\n")

    