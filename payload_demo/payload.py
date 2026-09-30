from flask import Flask, request
import datetime
import os

app = Flask(__name__)

# ဓာတ်ပုံများသိမ်းမည့် Folder ဆောက်ခြင်း
UPLOAD_FOLDER = 'received_payloads'
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

@app.route('/upload', methods=['POST'])
def upload_file():
    if request.data:
        # ပုံတစ်ပုံချင်းစီကို မထပ်အောင် အချိန်အတိုင်း နာမည်ပေးခြင်း
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"{UPLOAD_FOLDER}/payload_{timestamp}.jpg"
        
        # Binary Data ကို ဓာတ်ပုံအဖြစ် ပြန်သိမ်းခြင်း
        with open(filename, "wb") as f:
            f.write(request.data)
            
        print(f"[SUCCESS] Image received and saved as: {filename}")
        return "Image Saved Successfully", 200
    else:
        print("[ERROR] Received empty data")
        return "No Data Received", 400

if __name__ == '__main__':
    # Local Network တစ်ခုလုံးမှ ဝင်လာနိုင်ရန် host ကို '0.0.0.0' ထားပါသည်
    # Port 5000 တွင် Run ပါမည်
    app.run(host='0.0.0.0', port=5000, debug=True)