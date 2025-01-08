# Instrukcje tworzenia bindingów pod Pythona

## Tworzenie środowiska:
1. Wywołać skrypt zbiorczy - `install_minimal_env.sh` (`install_full_env.sh` powoduje dodatkowo instalację Doxygen'a i pakietu `v4l-utils`) lub zainstalować wybrane wersje pythonX.Y, pythonX.Y-dev i pythonX.Y-venv, np. `sudo apt install python3.6 python3.6-dev python3.6-venv`
20. Wywołać skrypt tworzący venv'y - `create_venvs.sh` (nie ma konieczności jeśli użyty został skrypt `install_*_env.py`)

## Budowanie wheel i instalacja:
1. Zbudować poleceniem `venv3xx/bin/python -m build --wheel`, w katalogu w którym znajdują się venv'y
20. Skopiować plik wheel w dowolne miejsce
30. Zainstalować w docelowym środowisku za pomocą pip

## Lub kompilacja i tymczasowa procedura instalacji:
1. Wywołać skrypt zbiorczy `build_python_binds.sh`
20. W przykładach z katalogu `examples` znajduje się skrypt `copy_kespp_tmp.sh`, który należy uruchomić aby zainstalować pakiet `kespp` w lokalizacji

## Development:
1. Zmiany nagłówków obsługiwanych przez binding do Pythona należy uwzględnić również w pliku stub, z rozszerzeniem `.pyi`

## Dostosowanie pod nową wersję Pythona:
1. Dopisać nową wersję do skryptu instalacji środowiska `install_minimal_env.sh`
20. Dopisać nową wersję venv do skryptu `create_venvs.sh` na wzór poprzednich
30. Przystosować build script (`build_python_binds.sh`) do aktualnej wersji Pythona (potrzebne ścieżki można podejrzeć za pomocą `python -m pybind11 --includes` (uruchamiane z venv'a)), zmienić wersję również w nazwie pliku wynikowego (x): `python3.x-config --extension-suffix`
40. Dopisać nowy skrypt do skryptu zbiorczego `build_all_python_binds.sh`
