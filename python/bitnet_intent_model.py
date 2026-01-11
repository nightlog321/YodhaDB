import torch
import torch.nn as nn
import torch.optim as optim
import random
import re
import json

# -------------------------------
# Config
# -------------------------------

VOCAB = [
    "show", "me", "users", "whose", "age",
    "is", "greater", "than", "less", "older",
    "younger", "above", "below", "equal", "to"
]

OPS = {
    ">": 0,
    "<": 1,
    "=": 2
}

IDX_TO_OP = {v: k for k, v in OPS.items()}

EMBED_DIM = 32
HIDDEN_DIM = 64
EPOCHS = 40
LR = 0.01

word_to_idx = {w: i for i, w in enumerate(VOCAB)}

# -------------------------------
# Tokenizer
# -------------------------------

def tokenize(text):
    tokens = re.findall(r"[a-z]+", text.lower())
    return [word_to_idx[t] for t in tokens if t in word_to_idx]

# -------------------------------
# Synthetic Data
# -------------------------------

def generate_sample():
    value = random.randint(18, 60)

    templates = [
        ("show users age greater than {}", ">"),
        ("users older than {}", ">"),
        ("age above {}", ">"),
        ("show users age less than {}", "<"),
        ("users younger than {}", "<"),
        ("age below {}", "<"),
        ("age equal to {}", "="),
        ("age is equal to {}", "=")
    ]

    template, op = random.choice(templates)
    return template.format(value), OPS[op]

def generate_dataset(n=2000):
    X, y = [], []
    for _ in range(n):
        text, op = generate_sample()
        X.append(tokenize(text))
        y.append(op)
    return X, y

# -------------------------------
# BitNet-Inspired Model
# -------------------------------

class BitNetIntentModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.embed = nn.Embedding(len(VOCAB), EMBED_DIM)
        self.fc1 = nn.Linear(EMBED_DIM, HIDDEN_DIM)
        self.fc2 = nn.Linear(HIDDEN_DIM, HIDDEN_DIM)
        self.op_head = nn.Linear(HIDDEN_DIM, len(OPS))

    def forward(self, x):
        emb = self.embed(x)
        h = emb.mean(dim=1)
        h = torch.relu(self.fc1(h))
        h = torch.relu(self.fc2(h))
        return self.op_head(h)

# -------------------------------
# BitNet-style binarization
# -------------------------------

def binarize(model):
    with torch.no_grad():
        for p in model.parameters():
            p.copy_(p.sign())

# -------------------------------
# Training
# -------------------------------

def train():
    X, y = generate_dataset()
    model = BitNetIntentModel()
    opt = optim.Adam(model.parameters(), lr=LR)
    loss_fn = nn.CrossEntropyLoss()

    for epoch in range(EPOCHS):
        total = 0
        for i in range(len(X)):
            x = torch.tensor(X[i], dtype=torch.long).unsqueeze(0)
            target = torch.tensor([y[i]])

            opt.zero_grad()
            out = model(x)
            loss = loss_fn(out, target)
            loss.backward()
            opt.step()

            total += loss.item()

        binarize(model)

        

    return model

# -------------------------------
# Inference
# -------------------------------

def infer(model, text):
    tokens = tokenize(text)
    x = torch.tensor(tokens, dtype=torch.long).unsqueeze(0)

    with torch.no_grad():
        logits = model(x)

    op = IDX_TO_OP[int(torch.argmax(logits))]

    # Deterministic numeric extraction
    m = re.search(r"\d+", text)
    if not m:
        raise ValueError("No numeric value found")

    value = int(m.group())

    return {
        "column": "user_age",
        "operator": op,
        "value": value
    }

# -------------------------------
# Demo
# -------------------------------

if __name__ == "__main__":
    import sys

    query = sys.argv[1]
    model = train()
    result = infer(model, query)
    print(json.dumps(result))