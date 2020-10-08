Część C - Węzły serwerowe zsynchronizowane

Rozwiązanie wzorowane na algorytmie Ricarta-Agrawali z synchronizacją Lamporta.
N - liczba serwerów w grupie w danym momencie

Obsługa żądania dodania pliku:
  
  Serwer dostaje żądanie dodania pliku
  Wysyła N-1 żądań o zgodę na dodanie pliku (do każdego serwera) i zaczyna czekać na komplet N-1 zgód.
  IF otrzymuje jakąkolwiek odmowę
      przestaje czekać na komplet zgód
      nie dodaje pliku
      wysyła NO_WAY do klienta
  ELSE IF dostaje N-1 zgód
      przestaje czekać na komplet zgód
      rezerwuje miejsce na serwerze
      wysyła CAN_ADD do klienta i przeprowadza standardową procedurę pobrania pliku 
  ELSE IF po upływie TIMEOUT sekund nie otrzymał kompletu zgód
      przestaje czekać na komplet zgód
      nie dodaje pliku
      wysyła NO_WAY do klienta

Obsługa żądania zgody na dodanie pliku:

  Serwer dostaje żądanie o zgodę na dodanie pliku
  IF plik o danej nazwie istnieje na serwerze
      serwer odsyła odmowę
  ELSE IF serwer czeka na komplet zgód na dodanie pliku
      IF wysłanie własnych żądań nastąpiło "wcześniej" niż odbiór żądania
          serwer odsyła odmowę
      ELSE IF wysłanie własnych żądań nastąpiło "później" niż czas odbioru żądania
          serwer odsyła zgodę
      ELSE IF para (IP unicast, port) serwera jest mniejsza od pary nadawcy
          serwer wysyła odmowę
      ELSE serwer wysyła zgodę
  ELSE serwer wysyła zgodę

Pomiar czasu:
1. Każdy węzeł utrzymuje lokalny licznik czasu.
2. Żądanie zgody na dodanie pliku zawiera stempel czasowy - lokalny czas wysłania rządania.
3. Lokalne liczniki "popychane" są w razie potrzeby przez przychodzące żądania (algorytm Lamporta). Po odebraniu żądania zgody na dodanie pliku węzeł ustawia swój licznik czasu na max(czas lokalny, czas żądania) + 1.

Dołączanie serwera do grupy:
1. Serwer dołącza się do grupy multicast.
2. Serwer wysyła komunikat "HELLO", aby poznać ilość dostępnego miejsca, adres IP unicast i numer portu każdego z serwerów w grupie. Komunikat ten zawiera też jego własne dane.
3. Serwer przegląda pliki znajdujące się w jego dzielonym folderze i próbuje je kolejno dodać do grupy zgodnie z protokołem opisanym powyżej. Jeżeli otrzyma zgodę na dodanie danego pliku to indeksuje go jako własny. Wpp. usuwa go z folderu dzielonego.  

Odłączanie serwera od grupy:
1. Serwer przed odłączeniem próbuje przesłać wszystkie swoje pliki do innego serwera. Wybiera je w kolejności od największej ilości zajętego miejsca. Korzysta z protokołu opisanego powyżej.
2. Serwer wysyła specjalny komunikat "BYE", aby poinformować serwery w grupie o swoim odłączeniu.
3. Serwer odłącza się od grupy multicast.

Szczegóły technicze:
1. Każdy serwer utrzymuje listę serwerów w grupie wraz z informacjami (IP unicast, numer portu, wolne miejsce).
2. Komunikaty "HELLO" i "BYE" służą aktualizacji tych list.
3. Przy wysyłaniu żądań zgody na dodanie pliku serwer kontroluje od jakich serwerów otrzymuje odpowiedzi. Ma to zapobiec szczególnym przypadkom, gdy do serwera zostanie wysłana zgoda, a ten odłączy się od grupy zanim ją otrzyma lub gdy serwer dostał żądanie zgody zanim się przywitał z resztą.
4. Żądania zgody na dodanie pliku posiadają numery sekwencyjne podobnie jak w części A+B, aby uniknąć błędnego reagowania na zagubione pakiety. 


