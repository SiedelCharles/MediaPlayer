import requests
from openai import OpenAI
import os
import re
# from pydub import AudioSegment

API_KEY = "pKJ06yxN5hMdCQcUCc0c8a11-9557-4864-908B-981d5240"
BASE_URL = "https://api.modelverse.cn/v1/"
UPLOAD_URL = "https://api.modelverse.cn/v1/audio/voice/upload"

def upload_custom_voice(
    audio_path: str,
    voice_name: str,
    model_name: str = "IndexTeam/IndexTTS-2",
    emotion_path: str = None
) -> str:
    files = {
        'speaker_file': open(audio_path, 'rb')
    }
    data = {
        'name': voice_name,
        'model': model_name
    }
    if emotion_path:
        files['emotion_file'] = open(emotion_path, 'rb')
    headers = {
        'Authorization': f'Bearer {API_KEY}'
    }
    try:
        response = requests.post(
            UPLOAD_URL,
            headers=headers,
            files=files,
            data=data
        )
        response.raise_for_status()
        result = response.json()
        voice_id = result.get('id')
        return voice_id
    except requests.exceptions.RequestException as e:
        if response.text:
            raise e 
        return None
    finally:
        for f in files.values():
            f.close()


def synthesize_with_custom_voice(
    voice_id: str,
    text: str,
    output_path: str,
    emotion_text: str,
    sample_rate: float, 
    emotion_weight: float = 0.8
):
    client = OpenAI(
        api_key=API_KEY,
        base_url=BASE_URL
    )
    params = {
        "model": "IndexTeam/IndexTTS-2",
        "voice": voice_id,
        "input": text,
    }
    if emotion_text:
        params["extra_body"] = {
            "emo_control_method": 3,  # 3=基于情绪文本
            "emo_text": emotion_text,
            "emo_weight": emotion_weight,
            "sample_rate": sample_rate
        }
    
    try:
        with client.audio.speech.with_streaming_response.create(**params) as response:
            response.stream_to_file(output_path)
    except Exception as e:
        raise e
  