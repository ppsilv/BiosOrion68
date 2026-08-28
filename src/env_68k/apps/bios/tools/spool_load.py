import time
import os
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import serial

PORTA_SERIAL = 'COM3'  # Ou /dev/ttyUSB0 no Linux
BAUD_RATE = 115200
PASTA_MONITORADA = "./orion_spool"

class OrionSpoolHandler(FileSystemEventHandler):
    def on_created(self, event):
        # Verifica se o arquivo gerado é o binário compilado
        if not event.is_directory and event.src_path.endswith('.bin'):
            print(f"\n[MAINFRAME] Novo arquivo detectado: {os.path.basename(event.src_path)}")
            self.enviar_para_orion(event.src_path)

    def enviar_para_orion(self, caminho_arquivo):
        # Pequeno delay para garantir que o compilador terminou de escrever o arquivo físico
        time.sleep(0.2) 
        
        try:
            print(f"[UART] Abrindo {PORTA_SERIAL} a {BAUD_RATE} baud...")
            ser = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=2)
            
            with open(caminho_arquivo, 'rb') as f:
                dados = f.read()
                tamanho = len(dados)
                print(f"[SPOOL] Transmitindo {tamanho} bytes para o Orion68K...")
                
                # Aqui você pode enviar um cabeçalho customizado se o seu VBUG2 precisar,
                # ou apenas jogar os bytes puros se o Orion já estiver esperando o dump.
                ser.write(dados)
                
            ser.close()
            print("[SUCESSO] Transmissão concluída! Aguardando próximo arquivo...")
            
            # Opcional: deletar ou mover o arquivo para uma pasta "enviados" para não retransmitir
            # os.remove(caminho_arquivo)
            
        except Exception as e:
            print(f"[ERRO] Falha na transmissão: {e}")

if __name__ == "__main__":
    if not os.path.exists(PASTA_MONITORADA):
        os.makedirs(PASTA_MONITORADA)
        
    event_handler = OrionSpoolHandler()
    observer = Observer()
    observer.schedule(event_handler, path=PASTA_MONITORADA, recursive=False)
    observer.start()
    
    print(f"=== Mainframe Orion68K Spooler Ativo ===")
    print(f"Monitorando a pasta: {PASTA_MONITORADA}")
    print("Apenas jogue o arquivo .bin lá dentro e veja a mágica acontecer. Ctrl+C para sair.")
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()