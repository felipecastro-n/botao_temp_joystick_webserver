from flask import Flask, render_template, request, jsonify
import json
import datetime

app = Flask(__name__)

# Armazenamento de dados
dados = {
    "button": False,
    "temperature": 0.0,
    "x": 0,
    "y": 0,
    "direcao": "Centro",  # Nova propriedade para a direção da rosa dos ventos
    "timestamp": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
}

# Histórico de leituras
historico = []

@app.route('/', methods=['GET', 'POST'])
def index():
    global dados
    
    if request.method == 'POST':
        try:
            # Recebe os dados JSON do Raspberry Pi Pico
            dados_recebidos = request.get_json(force=True)
            
            # Atualiza os dados com os valores recebidos
            dados["button"] = dados_recebidos.get("button", False)
            dados["temperature"] = dados_recebidos.get("temperature", 0.0)
            dados["x"] = dados_recebidos.get("x", 0)
            dados["y"] = dados_recebidos.get("y", 0)
            dados["direcao"] = dados_recebidos.get("direcao", "Centro")  # Processa a nova propriedade de direção
            dados["timestamp"] = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            
            # Adiciona ao histórico (limitando a 100 entradas)
            historico.append(dados.copy())
            if len(historico) > 100:
                historico.pop(0)
                
            print(f"Dados recebidos: {dados}")
            return jsonify({"status": "success"})
        except Exception as e:
            print(f"Erro ao processar dados: {e}")
            return jsonify({"status": "error", "message": str(e)}), 400
    
    # Para requisições GET, renderiza a página com os dados atuais
    return render_template('index.html', dados=dados, historico=historico)

@app.route('/api/dados', methods=['GET'])
def get_dados():
    return jsonify(dados)

@app.route('/api/historico', methods=['GET'])
def get_historico():
    return jsonify(historico)

# Rota para visualização da rosa dos ventos
@app.route('/rosa-dos-ventos', methods=['GET'])
def rosa_dos_ventos():
    return render_template('rosa_dos_ventos.html', direcao=dados["direcao"])

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
