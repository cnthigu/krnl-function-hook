# kernel-functio-hooking 

Demonstração educacional de **Function Hooking** em kernel Windows.

## A Função que Hookeamos

### `NtOpenCompositionSurfaceSectionInfo`

Esta função está localizada no driver `dxgkrnl.sys` (DirectX Graphics Kernel) e é responsável por operações relacionadas a superfícies de composição gráfica.

```assembly
# ANTES (função original):
NtOpenCompositionSurfaceSectionInfo:
    xor eax, eax
    ret

# DEPOIS (com nosso hook):
NtOpenCompositionSurfaceSectionInfo:
    mov rax, 0x1234567890ABCDEF  ; Endereço da nossa função
    jmp rax                       ; Pula para nossa função
```

![Hook Process](hook.png)

## Por que NtOpenCompositionSurfaceSectionInfo?

Esta função foi escolhida porque:

- Está no `dxgkrnl.sys` (DirectX Graphics Kernel)
- Raramente é chamada (menos chance de crash)
- É exportada (podemos encontrá-la facilmente)
- Funciona bem para comunicação usermode ↔ kernel
- Boa para fins educacionais

### Outras funções que funcionam:
- `NtOpen*` (maioria das funções NtOpen)
- Funções com "Composition" no nome
- Lista completa: https://j00ru.vexillium.org/syscalls/win32k/64/

### Funções para evitar:
- Funções com "SecureCookie" (causam BSOD)
- Funções em regiões críticas do sistema

## Como Funciona

![Hook Visualization](hook.png)

1. **Driver** substitui os primeiros bytes da função por shellcode
2. **Shellcode** redireciona execução para nossa função `hook_handle`
3. **User Mode** chama a função hookada para operações de memória
4. **Kernel** processa a requisição e retorna resultado

## 📸 Onde Colocar a Print?



## Como Executar

### 1. Compilar
```bash
# Abrir kernel_mode.sln no Visual Studio
# Build Solution (Ctrl+Shift+B)
# Projeto configurado apenas para Release x64
```

### 2. Executar
```bash
# 1. Abrir DebugView como administrador
# 2. Carregar driver
kdmapper.exe kernel_mode.sys

# 3. Abrir notepad
notepad

# 4. Executar demo
user_mode.exe
```

## Saída Esperada

### DebugView (Kernel Logs):
```
[KERNEL] Driver carregado com sucesso!
[KERNEL] Iniciando instalacao do hook...
[KERNEL] Hook instalado com sucesso!
[KERNEL] Interceptando: NtOpenCompositionSurfaceSectionInfo
[KERNEL] Solicitando endereco base: notepad.exe (PID: 1234)
[KERNEL] Endereco base encontrado: 0x7ff6a0000000
[KERNEL] Operacao de LEITURA em 0x7ff6a0001000 (tamanho: 4)
[KERNEL] Operacao de ESCRITA em 0x7ff6a0001000 (tamanho: 4)
```

### Terminal (User Mode):
```
=== Kernel Mode Hook Demo ===
Driver: kernel_mode.sys
Hook: NtOpenCompositionSurfaceSectionInfo

[OK] user32.dll carregado
[OK] Notepad encontrado! PID: 1234
[OK] Endereco base: 0x7ff6a0000000

=== Demonstracao ===
Lendo 0x7ff6a0001000 = 0
Escrevendo 12345 em 0x7ff6a0001000

=== Concluido! ===
Hook funcionando corretamente!
```

## Estrutura do Projeto

```
kernel_mode/
├── kernel_mode/          # Driver kernel
│   ├── main.cpp         # DriverEntry + logs
│   ├── hook.cpp/h       # Function hooking
│   ├── memory.cpp/h     # Operações de memória
│   └── kernel_mode.inf  # Arquivo do driver
├── user_mode/           # Aplicação user mode
│   └── main.cpp         # Demo simples
└── kernel_mode.sln      # Solução Visual Studio
```


*Demonstração simples de function hooking em kernel mode! *