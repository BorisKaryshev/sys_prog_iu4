# Задача

Ротация логов

Сигнатура скрипта:
```
./script.sh [OPTION...] <config_path>
    -a, --algoritm <alg>        compression algorithm

Config path can be relative and absolute
```

На выход ничего не подаем

В конфиге храним:

- target src (paths)
- target dst (path to dir)
- N максимальное хранилище бэкапов (после того как накопилось > N бэкапов мы удалем старые)
- policy (удалять старые или новые)

Файлики из paths мы кладем в архив и его в target-dst
