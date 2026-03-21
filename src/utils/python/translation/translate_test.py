from translate import process_srt, translate, combination

# print("start\n")
# process_srt("D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/src/utils/python/translation/test.txt",
#     "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/src/utils/python/translation/timestamp.txt",
#     "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/src/utils/python/translation/processed.txt"
# )
# print("end\n")

print("start")
translate("D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/src/utils/python/translation/processed.txt",
    "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/src/utils/python/translation/translated.txt"
)
print("end")

# process_srt(r"D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\MediaPlayer\\src\\core\\python\\translation\\timestap.txt",
#     r"D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\MediaPlayer\\src\\core\\python\\translation\\translated.txt",
#     r"D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\MediaPlayer\\src\\core\\python\\translation\\final.txt"
# )