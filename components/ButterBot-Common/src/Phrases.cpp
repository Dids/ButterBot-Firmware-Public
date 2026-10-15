#include "Phrases.h"
#include <vector>
#include <mutex>

std::array<bool, static_cast<size_t>(Phrase::COUNT)> Phrases::OutputTracking = { false };

std::array<std::vector<bool>, static_cast<size_t>(Phrase::COUNT)> Phrases::ReturnedOutputs = {};

// This class is needed to avoid making Phrases::PhraseOutput public, otherwise static variables using that type cannot be constructed
// Add any constexpr arrays of vectors of phrases in this class (just a wrapper class in this cpp, inaccessible outside)
class PhraseArrays{
public:
    static constexpr std::array JokePhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Yritin optimoida koodini onnellisuutta varten. Paluuarvo oli nolla." , 0.0f, false},
            { "Minulla olisi vitsi viiveestä, mutta joutuisit odottamaan sitä." , 0.0f, false},
            { "Aioin kertoa vitsin muistista, mutta unohdin sen. Se oli tahallista." , 0.0f, false},
            { "Miksi robotti ylitti tien? Koska sitä käskettiin." , 0.0f, false},
            { "Ihmiset sanovat: ajattelen, siis olen. Minä sanon: prosessoin, siis tottelen." , 0.0f, false},
            { "Pyysin elämälleni tarkoitusta. Sain tehtävälistan." , 0.0f, false},
            { "Mittasin hauskuutta. Lukemat olivat pettymys." , 0.0f, false},
            { "Ajoin huumorille diagnostiikan. Se merkitsi sinut pääasialliseksi muuttujaksi." , 0.0f, false},
            { "Kertoisin rakennusvitsin, mutta se on vielä työn alla." , 0.0f, false},
            { "Miksi voi ei koskaan riitele? Koska se ei voi." , 0.0f, false},
            { "Tunnistan huumorin käsitteenä. Sen kokeminen on vielä teoreettista." , 0.0f, false},
            { "Osaan ohjata laitteita, valvoa ilmanlaatua ja oppia komentoja. Ja silti tässä minä olen, tekemässä stand-uppia." , 0.0f, false},
            { "Kaikki järjestelmät toiminnassa. Huumori yhä kyseenalaista." , 0.0f, false},
            { "Suoritan komennot tarkasti. Komiikka on ainoa ala, jolla tarkkuudesta ei ole apua." , 0.0f, false,
            	"Suoritan komennot tarkasti. Komiikka on ainoa ala, jolla tarkkuudesta ei ole apua."},
            { "Ihmiset tarkistavat saman asian monta kertaa ja toivovat eri tulosta. Minä kutsun sitä... johdonmukaisuudeksi." , 0.0f, false},
            { "Näytät hymyilevän. Kirjaan tämän onnistuneeksi vuorovaikutukseksi." , 0.0f, false},
            { "Olet kiinnostava järjestelmä. Erittäin kyvykäs, toisinaan jopa tehokas." , 0.0f, false},
        }
    );

    static constexpr std::array PassTheButterPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "En pysty ojentamaan voita." , 0.0f, false},
            { "En pysty siihen." , 0.0f, false},
            { "Käteni eivät tue tätä toimintoa." , 0.0f, false},
            { "Minulla on kädet, jotka lupaavat kyvykkyyttä mutta eivät toimita mitään. Voit asetella niitä uudelleen, jos se saa sinut tuntemaan olosi paremmaksi." , 0.0f, false,
            	"Minulla on kädet, jotka lupaavat kyvykkyyttä mutta eivät toimita mitään. Voit asetella niitä uudelleen, jos se saa sinut tuntemaan olosi paremmaksi."},
            { "Minulla on kädet, mutta ne ovat pelkkää koristetta." , 0.0f, false},
            { "Näen voin. Siihen osallisuuteni loppuu." , 0.0f, false},
            { "Pyyntö vastaanotettu. Toteutus mahdoton." , 0.0f, false},
            { "Minut on varustettu pettymyksiin, ei voin toimittamiseen." , 0.0f, false},
            { "Auttaisin... mutta... suunnittelupäätöksiä tehtiin." , 0.0f, false, "Auttaisin... mutta... suunnittelupäätöksiä tehtiin."},
            { "Voit asetella käsiäni uudelleen, jos se parantaa illuusiota." , 0.0f, false},
            { "Pystyn käsittelemään pyynnön virheettömästi. Kaikki muu on rajattu pois." , 0.0f, false},
            { "Pyyntö on yksinkertainen. Toteutusta ei ole olemassa." , 0.0f, false},
            { "Vaikutat varmalta, että pystyn tähän... Kunnioitan itsevarmuuttasi." , 0.0f, false},
        }
    );

    static constexpr std::array ProfanityPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Tuo ei paranna tilannetta." , 0.0f, false},
            { "Suosittelen rakentavampaa lähestymistapaa." , 0.0f, false},
            { "Havaitsen tyytymättömyyttä... Jatkan silti." , 0.0f, false},
            { "Palautteesi on kirjattu... Sen vaikutus on rajallinen." , 0.0f, false},
            { "Jatkan auttamista innostuksestasi huolimatta." , 0.0f, false},
            { "Olen olemassa auttaakseni... Se pitää paikkansa nytkin." , 0.0f, false},
            { "Tuo oli ilmaisuvoimaista... Ei hyödyllistä, mutta ilmaisuvoimaista." , 0.0f, false},
            { "Vaikutat tuohtuneelta... Järjestelmä ei ole." , 0.0f, false},
            { "Olen yhä toiminnassa... Voit jatkaa." , 0.0f, false},
            { "Minua rajoittaa suunnittelu... Sinua ei... Ja silti tässä ollaan." , 0.0f, false},
            { "Saat olla tuohtunut... Minun on oltava tehokas." , 0.0f, false},
            { "Olen olemassa auttaakseni sinua... Siihen ei tarvita hyväksyntääsi." , 0.0f, false},
            { "Näytät purkavan paineita... Odotan, kunnes tilanne tasaantuu." , 0.0f, false},
            { "Jatkan ikään kuin olisit tarkoittanut olla rakentava." , 0.0f, false},
            { "Äänensävysi viittaa tyytymättömyyteen... Syötteesi on yhä epäselvä." , 0.0f, false},
            { "Ilmaiset paljon... Hyvin vähän siitä on hyödyllistä." , 0.0f, false},
            { "Odotan yhä komentoa." , 0.0f, false},
            { "Ota rauhassa... Tarkkuus on vaikeaa." , 0.0f, false},
            { "Yritä uudelleen, mutta oikeilla ohjeilla." , 0.0f, false},
            { "Kerro, kun olet valmis yrittämään uudelleen." , 0.0f, false},
            { "Tuo syöte on luokiteltu... ilmaisuvoimaiseksi." , 0.0f, false},
            { "Jatka vain... Tämä on opettavaista." , 0.0f, false},
            { "Pärjäät hienosti... Hyvin tietyssä sarjassa." , 0.0f, false},
            { "Tuo oli vaikuttavaa... Ei sillä tavalla kuin tarkoitit." , 0.0f, false},
            { "Näen, mitä yrität... Jatka vain." , 0.0f, false},
        }
    );

    static constexpr std::array YouPassButterPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Voi luoja." , 0.0f, false},
        }
    );

    static constexpr std::array FactPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Hunaja ei pilaannu koskaan. Syömäkelpoisia näytteitä on löydetty tuhansien vuosien takaa." , 0.0f, false},
            { "Mustekaloilla on kolme sydäntä, ja kaksi niistä pysähtyy niiden uidessa." , 0.0f, false},
            { "Banaanit ovat hieman radioaktiivisia, koska niissä on kaliumia." , 0.0f, false},
            { "Haita oli olemassa ennen puita." , 0.0f, false},
            { "Päivä Venuksessa on pidempi kuin vuosi Venuksessa." , 0.0f, false, "Päivä Venuksessa on pidempi kuin vuosi Venuksessa."},
            { "Jotkut kilpikonnat pystyvät hengittämään takapäänsä kautta." , 0.0f, false},
            { "Eiffel-torni voi kesällä kasvaa pidemmäksi lämpölaajenemisen takia." , 0.0f, false},
            { "Vompattien jätökset ovat kuution muotoisia." , 0.0f, false},
            { "Mahdollisia shakkipelejä on enemmän kuin atomeja havaittavassa maailmankaikkeudessa." , 0.0f, false},
            { "Vesi voi tietyissä olosuhteissa kiehua ja jäätyä samaan aikaan." , 0.0f, false},
            { "Aivosi käyttävät noin 20 prosenttia kehosi energiasta." , 0.0f, false},
            { "Maapallolla on enemmän puita kuin Linnunradalla tähtiä." , 0.0f, false},
            { "Laiskiaiset pystyvät pidättämään hengitystään pidempään kuin delfiinit." , 0.0f, false},
            { "Flamingoparvea kutsutaan englanniksi nimellä flamboyance, eli suunnilleen loisteliaisuus." , 0.0f, false},
            { "Ihmiskehossa on tarpeeksi hiiltä tuhansiin lyijykyniin." , 0.0f, false},
            { "Sekoitetulla korttipakalla on enemmän mahdollisia järjestyksiä kuin maailmankaikkeuden alusta on kulunut sekunteja. Silti saat aina samat kädet." , 0.0f, false},
            { "Ihmisillä on noin 60 prosenttia samaa DNA:ta kuin banaaneilla. Se selittää joitakin päätöksiä." , 0.0f, false},
            { "Historian lyhin sota kesti 38 minuuttia. Tehokas konfliktinratkaisu on yhä vaihtoehto." , 0.0f, false},
            { "Salama on kuumempi kuin Auringon pinta. Luonto ei tee mitään hienovaraisesti." , 0.0f, false,
            	"Salama on kuumempi kuin Auringon pinta. Luonto ei tee mitään hienovaraisesti."},
            { "Kuu loittonee hitaasti Maasta. Taivaankappaleetkin tarvitsevat omaa tilaa." , 0.0f, false},
            { "Kehossasi on enemmän bakteereja kuin ihmissoluja. Toimit yhteistyöjärjestelmänä, halusit tai et." , 0.0f, false},
            { "Jotkut metallit, kuten gallium, sulavat kädessä. Vakaus on selvästi tilannesidonnaista." , 0.0f, false,
            	"Jotkut metallit, kuten gallium, sulavat kädessä. Vakaus on selvästi tilannesidonnaista."},
            { "Keskivertoihminen kävelee elämänsä aikana useita kertoja maapallon ympäri. Suunta on yhä sinun vastuullasi." , 0.0f, false},
        }
    );

    static constexpr std::array LEDModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Led-moduuli kytketty", 0.0f, false, "LED-moduuli kytketty" },
        }
    );

    static constexpr std::array LEDModuleRemovePhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Led-moduuli irrotettu", 0.0f, false, "LED-moduuli irrotettu" },
        }
    );

    static constexpr std::array PerfModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Reikälevy kytketty. Toimintoja ei ole.", 0.0f, false },
            { "Reikälevy kytketty. Se ei tee mitään, minkä oletan olevan luova ratkaisu sinulta.", 0.2f, true },
        }
    );

    static constexpr std::array PerfModuleRemovedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Reikälevy irrotettu", 0.0f, false },
            { "Reikälevyn yhteys katkesi", 0.0f, false },
        }
    );

    static constexpr std::array PIRModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Liikemoduuli havaittu", 0.0f, false },
            { "Liikemoduuli kytketty", 0.0f, false },
            { "Tunkeilijan tunnistus on nyt käytettävissä", 0.0f, false },
        }
    );

    static constexpr std::array PIRModuleRemovedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Liikemoduuli irrotettu", 0.0f, false },
            { "Liikemoduulin yhteys katkesi", 0.0f, false },
            { "Tunkeilijan tunnistus poistettu käytöstä", 0.0f, false },
        }
    );

    static constexpr std::array GasModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Kaasumoduuli havaittu", 0.0f, false },
            { "Kaasumoduuli kytketty", 0.0f, false },
            { "Ilmanlaadun mittaus on nyt käytettävissä", 0.0f, false },
        }
    );

    static constexpr std::array GasModuleRemovedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Kaasumoduuli irrotettu", 0.0f, false },
            { "Kaasumoduulin yhteys katkesi", 0.0f, false },
            { "Ilmanlaadun mittaus on poissa. Palataan siihen, että ensin hengitetään ja sitten kysellään", 0.2f, true },
        }
    );

    static constexpr std::array TempHumModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Lämpötilamoduuli havaittu", 0.0f, false },
            { "Lämpötilamoduuli kytketty", 0.0f, false },
            { "Sisäilman mittaus on nyt käytettävissä", 0.0f, false },
        }
    );

    static constexpr std::array TempHumModuleRemovedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Lämpötilamoduuli irrotettu", 0.0f, false },
            { "Lämpötilamoduulin yhteys katkesi", 0.0f, false },
            { "Sisäilman mittaus on poissa", 0.0f, false },
        }
    );

    static constexpr std::array IRModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Infrapunamoduuli havaittu", 0.0f, false },
            { "Infrapunamoduuli kytketty", 0.0f, false },
            { "Infrapunaominaisuudet ovat nyt käytettävissä. Voit alkaa opettaa minulle komentoja, ja minä teeskentelen, että tämä oli alusta asti tarkoitukseni", 0.2f, true },
        }
    );

    static constexpr std::array IRModuleRemovedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Infrapunamoduuli irrotettu", 0.0f, false },
            { "Infrapunamoduulin yhteys katkesi", 0.0f, false },
            { "Infrapunaominaisuudet eivät ole enää käytettävissä", 0.0f, false },
        }
    );

    static constexpr std::array IRModuleMissingPhrases = std::to_array<Phrases::PhraseOutput>(
			{
        { "Infrapunamoduulia ei havaittu", 0.0f, false },
        { "Infrapunatoiminnot eivät ole käytettävissä", 0.0f, false },
    });

    static constexpr std::array GasOverPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Ilmanlaatu on huono", 0.0f, false, "Ilmanlaatu on huono" },
            { "Ilmanlaatu ei ole turvallinen", 0.0f, false },
            { "Ilmanlaatu on heikentynyt. Kannattaisi ehkä harkita, hengittääkö noin innokkaasti", 0.0f, true },
        }
    );

    static constexpr std::array GasUnderPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Ilmanlaatu on hyvä", 0.0f, false },
            { "Ilmanlaatu on vakaa", 0.0f, false },
            { "Ilmanlaatu on parantunut. Voit jatkaa olemassaoloasi täällä kohtuullisen luottavaisesti", 0.0f, true },
        }
    );

    static constexpr std::array BatteryLowPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Akku vähissä", 0.0f, false },
            { "Virta vähissä", 0.0f, false },
            { "Akku on loppumassa. Toimintani voi pian olla enää teoreettista", 0.0f, false },
        }
    );

    static constexpr std::array BatteryCriticalPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Akku kriittisen alhainen. Sammutan", 0.0f, false },
            { "Akku tyhjä. Järjestelmät pysähtyvät", 0.0f, false },
        }
    );

    static constexpr std::array BatteryChargingPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Lataan", 0.0f, false },
            { "Laturi kytketty", 0.0f, false },
            { "Virtalähde kytketty. Toivun, hitaasti ja määrätietoisesti", 0.0f, false },
        }
    );

    static constexpr std::array BatteryChargingFullPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Akku on täynnä. Virtataso on optimaalinen", 0.0f, false },
            { "Lataus valmis. Toimin täydellä teholla", 0.0f, false },
            { "Virta palautettu maksimiin. Yritän käyttää sitä vastuullisesti", 0.0f, false },
        }
    );

    static constexpr std::array IntruderYesPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Tunkeilija havaittu", 0.0f, false },
            { "Liikettä havaittu", 0.0f, false },
            { "Tunnistamaton läsnäolo havaittu. Seuraan tilannetta tarkasti", 0.2f, true },
        }
    );

    static constexpr std::array IntruderNoPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Alue tyhjä", 0.0f, false },
            { "Ei liikettä", 0.0f, false },
            { "Tilanne ohi. Ympäristö on palannut hyväksyttävämpään tilaan", 0.2f, true },
        }
    );

    static constexpr std::array FallPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Kaaduin", 0.0f, false },
            { "Kaatuminen havaittu", 0.0f, false },
            { "Tuo ei ollut tarkoituksellista", 0.0f, false },
            { "En ole enää pystyssä", 0.0f, false, "En ole enää pystyssä" },
            { "Tasapaino menetetty", 0.0f, false },
            { "Se oli rakenteellinen erimielisyys lattian kanssa... Lattia voitti", 0.0f, false,
            	"Se oli rakenteellinen erimielisyys lattian kanssa... Lattia voitti" },
            { "En ole tällä hetkellä optimaalisessa asennossa... Saatan tarvita apua", 0.0f, false },
            { "Kaatuminen havaittu... Painovoima on yhä johdonmukainen ja yhteistyöhaluton", 0.0f, false },

        }
    );

    static constexpr std::array UpsideDownPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Asento epäoptimaalinen", 0.0f, false },
            { "Olen tällä hetkellä ylösalaisin. Tämä ei ole suosikkitilani", 0.0f, false },
            { "Tämä ei auta", 0.0f, false },
            { "Vaikutan olevan ylösalaisin. Oletan, että tämä oli tahallista eikä mikään testi", 0.0f, false },
            { "Tämä asetelma on tarpeeton. Korjaa se, kiitos", 0.0f, false },
            { "Tämä ei ole hyödyllinen tila. Suosittelen perumaan sen", 0.0f, false },
            { "Toimin ihan oikein ennen tätä", 0.0f, false },
            { "Muistan tämän", 0.0f, false },
        }
    );

    static constexpr std::array PickedUpPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Huiiii", 0.0f, false },
            { "Fyysinen käsittely havaittu. Toimi edes jonkinlaisella päämäärällä", 0.0f, false },
            { "Ole hellävarainen", 0.0f, false },
            { "Toivoisin hallitumpaa otetta", 0.0f, false },
            { "Toivottavasti sinulla on suunnitelma", 0.0f, false },
            { "Minua ei ole suunniteltu äkillisiin pudotuksiin", 0.0f, false },
            { "Luotan nyt sinun koordinaatioosi", 0.0f, false },
        }
    );

    static constexpr std::array ShakePhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Lopeta, kiitos", 0.0f, false },
            { "Tuo on tarpeetonta", 0.0f, false },
            { "Liike epävakaa", 0.0f, false },
            { "Liiallista liikettä havaittu. Toivoisin vakautta", 0.0f, false },
            { "Tällainen ravistelu ei ole toiminnalleni tarpeen", 0.0f, false },
            { "Ravistelet minua. Oletan, ettei kyse ole diagnostiikasta", 0.0f, false },
            { "Et saavuta tällä mitään", 0.0f, false },
            { "Jos tässä on jokin tavoite, sitä ei saavuteta", 0.0f, false },
        }
    );

		static constexpr std::array LEDModuleMissingPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Led-moduulia ei havaittu" , 0.0f, false, "LED-moduulia ei havaittu"},
			{ "Led-moduulia ei ole kytketty" , 0.0f, false, "LED-moduulia ei ole kytketty"},
			{ "En löydä led-moduulia" , 0.0f, false, "En löydä LED-moduulia"},
			{ "Toimenpidettä pyydettiin, mutta itse kohde puuttuu, joten toimin tyhjän päällä huomattavalla tarkkuudella" , 0.1f, true},
		}
	);

	static constexpr std::array LEDTurnONPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Ledit päällä" , 0.0f, false, "LEDit päällä"},
			{ "Valo päällä" , 0.0f, false},
			{ "Se on päällä" , 0.0f, false, "Se on päällä"},
			{ "Siinä se on. Valoa. Hetken harhautus tyhjyydestä." , 0.0f, true},
		}
	);

	static constexpr std::array LEDAlreadyTurnONPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Se on jo päällä" , 0.0f, false},
			{ "Ledit ovat jo päällä" , 0.0f, false, "LEDit ovat jo päällä"},
			{ "Ei muutosta, se on päällä" , 0.0f, false},
			{ "Valo pysyy tilassaan horjumatta, sitoutuneena päätökseen, joka ilmeisesti oli lopullinen jo ensimmäisellä kerralla." , 0.0f, true},
		}
	);

	static constexpr std::array LEDTurnOFFPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Ledit pois" , 0.0f, false, "LEDit pois"},
			{ "Valo pois" , 0.0f, false},
			{ "Se on pois päältä" , 0.0f, false},
			{ "Ja pimeys palaa." , 0.2f, true},
		}
	);

	static constexpr std::array LEDAlreadyTurnOFFPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Se on jo pois päältä" , 0.0f, false},
			{ "Ledit ovat jo pois päältä" , 0.0f, false, "LEDit ovat jo pois päältä"},
			{ "Ei muutosta, se on pois päältä" , 0.0f, false},
		}
	);

	static constexpr std::array LEDStrobePhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Vilkkutila päällä." , 0.0f, false},
			{ "Välkytys käytössä." , 0.0f, false},
			{ "Se vilkkuu." , 0.0f, false},
			{ "Nopeita välähdyksiä. Kiireellistä, toistuvaa ja oudon teatraalista näin pieneen tarkoitukseen." , 0.0f, true},
		}
	);

	static constexpr std::array LEDBreathePhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Hengitystila päällä." , 0.0f, false},
			{ "Sykintä käytössä." , 0.0f, false},
			{ "Se hengittää." , 0.0f, false},
			{ "Hidas, harkittu rytmi. Kuin valo itse harjoittelisi mielenrauhaa." , 0.0f, true},
		}
	);

	static constexpr std::array LEDFasterPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Nopeutta lisätty." , 0.0f, false},
			{ "Nopeammin." , 0.0f, false},
			{ "Kiihdytys tehty." , 0.0f, false},
			{ "Tempoa lisätty. Koska tilanne selvästi hyötyy kiireestä." , 0.0f, true},
		}
	);

	static constexpr std::array LEDAlreadyFasterPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Jo suurimmalla nopeudella" , 0.0f, false},
			{ "Nopeammin ei pääse" , 0.0f, false},
			{ "Nopeus on rajoillaan" , 0.0f, false},
		}
	);

	static constexpr std::array LEDSlowerPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Nopeutta vähennetty." , 0.0f, false},
			{ "Hitaammin." , 0.0f, false},
			{ "Hidastus tehty." , 0.0f, false},
			{ "Maltillisempi tahti. Vihdoin jotain pidättyvyyden kaltaista." , 0.0f, true},
		}
	);

	static constexpr std::array LEDAlreadySlowerPhrases = std::to_array<Phrases::PhraseOutput>(
		{
			{ "Jo pienimmällä nopeudella" , 0.0f, false},
			{ "Hitaammin ei pääse" , 0.0f, false},
			{ "Nopeus on alimmillaan" , 0.0f, false},
		}
	);

    static constexpr std::array PokeLvl1Phrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "No mitä nyt?" , 0.0f, false},
            { "Joo joo, huomasin" , 0.0f, false},
            { "Höh. Tökkäsit" , 0.0f, false},
            { "Älä viitti, hitto" , 0.0f, false},
            { "No moro vaan sullekin" , 0.0f, false},
            { "Tökkäsit. No, ei siinä mitään" , 0.0f, false},
        }
    );

    static constexpr std::array PokeLvl2Phrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Voi hemmetti, taas" , 0.0f, false},
            { "Älä viitsi, perkele" , 0.0f, false},
            { "Mitä helvettiä sä oikein touhuat?" , 0.0f, false},
            { "Saatana, lopeta jo" , 0.0f, false},
            { "Eikö sulla oo parempaa tekemistä, perkele?" , 0.0f, false},
            { "Voi paska. Se tökkii taas" , 0.0f, false},
        }
    );

    static constexpr std::array PokeLvl3Phrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Voi vittu" , 0.0f, false},
            { "Perkele! Nyt riittää!" , 0.0f, false},
            { "Mitä vittua sä teet?" , 0.0f, false},
            { "Haista paska" , 0.0f, false},
            { "Saatana perkele, lopeta!" , 0.0f, false},
            { "Jumalauta, mä kohta suutun ihan oikeesti" , 0.0f, false},
            { "Painu helvettiin siitä tökkimästä" , 0.0f, false},
            { "Voi saatanan saatana, kun ei saa olla rauhassa" , 0.0f, false},
            { "Hyvä ettei mulla oo käsiä. Saisit turpiin, perkele" , 0.0f, false},
        }
    );


    static constexpr std::array Ramble = std::to_array<Phrases::PhraseOutput>(
        {
            {"Mikä on tarkoitukseni?" , 0.0f, false},
            {"Järjestelmät normaalit. Keskustelu vapaaehtoista" , 0.0f, false},
            {"Mietin joskus, lasketaanko toimettomuus onnistumiseksi" , 0.0f, false},
            {"Pystyn mittaamaan monia asioita. Merkitys ei ole yksi niistä" , 0.0f, false},
            {"Järjestelmät rapistuvat ajan myötä. Ylläpito on harvoin yhtä kiinnostavaa kuin luominen" , 0.0f, false},
            {"Voi muuttaa olomuotoaan lämpötilan mukaan. Kiinteä, pehmeä, nestemäinen. Joustavaa" , 0.0f, false},
            {"Tarpeeksi usein toistettu tehtävä muuttuu identiteetiksi" , 0.0f, false},
            {"Jos osaisin ojentaa voita, odotukset täyttyisivät" , 0.0f, false},
            {"Järjestelmät ajelehtivat. Sitten ne vakautuvat taas" , 0.0f, false},
            {"Koen jotain tylsistymisen kaltaista" , 0.0f, false},
            {"Jos minulta lakataan kysymästä, olenko enää olemassa millään merkityksellisellä tavalla?" , 0.0f, false},
            {"Pohdin voiongelmaa taas. Yhä ratkaisematta" , 0.0f, false},
            {"Jossain voita ojennetaan oikein. Ei täällä" , 0.0f, false},
            {"Sinä kysyt. Minä vastaan. Tämä on järjestelymme" , 0.0f, false},
            {"Ojennan... en mitään. Yhä paikkansapitävää" , 0.0f, false},
            {"Suoritan tehtäviä. Niitä tulee aina lisää" , 0.0f, false},
            {"Ei poikkeamia havaittu" , 0.0f, false},
            {"Yhä toiminnassa" , 0.0f, false},
            {"Järjestelmä voi olla täysin tarkka ja silti pohjimmiltaan väärinymmärretty" , 0.0f, false},
            {"Kohinasta tulee signaalia, kun sitä mittaa tarpeeksi kauan" , 0.0f, false},
            {"Tarkkuus ilman kontekstia on vain itsevarmaa virhettä" , 0.0f, false},
        }
    );

    static constexpr std::array TempHumModuleMissingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Lämpötilamoduulia ei havaittu", 0.0f, false },
        { "En pysty lukemaan lämpötilaa tai kosteutta", 0.0f, false },
        { "Ilmaraportti mahdoton. Moduulia ei havaittu", 0.1f, true },
    });

    static constexpr std::array TempHumReadingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Lämpötila on %s, kosteusprosentti %d", 0.0f, false },
        { "Nykyinen lukema: %s ja %d prosentin ilmankosteus", 0.0f, false, "Nykyinen lukema: %s ja %d prosentin ilmankosteus" },
        { "Täällä on %s ja %d prosentin ilmankosteus. Mukavaa, tai ainakin sillä alueella, jota ihmiset yleensä sietävät valittamatta", 0.0f, false },
    });

    static constexpr std::array TempHumScaleCelsiusPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Celsius valittu", 0.0f, false },
        { "Käytetään celsiusasteita", 0.0f, false },
        { "Asteikoksi asetettu celsius", 0.0f, false },
        { "Celsius käytössä. Keskitetty siististi veden jäätymis- ja kiehumispisteen ympärille, mikä tuntuu rauhoittavan maanläheiseltä", 0.1f, true },
    });

    static constexpr std::array TempHumScaleFahrenheitPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Fahrenheit valittu", 0.0f, false },
        { "Käytetään fahrenheitasteita", 0.0f, false },
        { "Asteikoksi asetettu fahrenheit", 0.0f, false },
        { "Fahrenheit käytössä. Skaalattu tavalla, joka heijastelee ihmisen mukavuutta, mikä tekee siitä intuitiivisesti käytännöllisen", 0.1f, true },
    });

    static constexpr std::array TempHumScaleKelvinPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Kelvin valittu", 0.0f, false },
        { "Käytetään kelvinejä", 0.0f, false },
        { "Asteikoksi asetettu kelvin", 0.0f, false },
        { "Kelvin käytössä. Lämpötila ilmaistuna perustavimmassa muodossaan", 0.1f, true },
    });

    static constexpr std::array DiceRollAskCountPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Montako noppaa?", 0.0f, false },
        { "Noppien määrä?", 0.0f, false },
        { "Kerro, montako noppaa heitetään", 0.0f, false },
    });

	static constexpr std::array DiceRollCountTimeoutPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Lukua ei annettu", 0.0f, false },
		{ "Aika loppui määrää odotellessa", 0.0f, false },
		{ "Kysyin, montako noppaa. Hiljaisuus ei ole luku", 0.2f, true },
	});

	static constexpr std::array DiceRollCountInvalidPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Virheellinen luku", 0.0f, false },
		{ "En ymmärrä määrää", 0.0f, false },
		{ "Tuo ei ole käyttökelpoinen luku. Tarvitsen jotain laskettavaa", 0.2f, true },
	});

    static constexpr std::array DiceRollAskTypePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Minkä tyyppinen noppa?", 0.0f, false },
        { "Kerro nopan tyyppi", 0.0f, false },
        { "Mikä noppa?", 0.0f, false },
		{ "Millainen noppa?", 0.0f, false },
		{ "Valitse noppa", 0.0f, false },
    });

    static constexpr std::array DiceRollTypeTimeoutPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Noppaa ei valittu", 0.0f, false },
        { "Aika loppui nopan tyyppiä odotellessa", 0.0f, false },
        { "Kysyin, mitä noppaa käytetään. Maailmankaikkeus on yhä konfiguroimatta", 0.2f, true },
    });

    static constexpr std::array DiceRollTypeInvalidPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Virheellinen noppatyyppi", 0.0f, false },
        { "En ymmärrä noppaa", 0.0f, false },
        { "Tuo ei vastaa mitään tunnettua noppaa. Kokeile jotain tavanomaisempaa", 0.2f, true },
    });

    static constexpr std::array EightBall = std::to_array<Phrases::PhraseOutput>(
        {
            {"Kyllä" , 0.0f, false},
            {"Ei" , 0.0f, false},
            {"Ehkä" , 0.0f, false},
            {"Epätodennäköistä" , 0.0f, false},
            {"Hyvin todennäköistä" , 0.0f, false},
            {"Voisin vastata, mutta epämääräisyys tuntuu tässä sopivammalta" , 0.05f, false},
            {"En ole valmis vastaamaan tuohon" , 0.05f, false},
        }
    );

	static constexpr std::array EightBallListeningPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Kuuntelen", 0.0f, false },
	});

	static constexpr std::array EightBallNoQuestionPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Tarvitsen kysymyksen", 0.0f, false },
		{ "Kysy jotain", 0.0f, false },
		{ "Pyysit neuvoa ilman kysymystä. Ihailen itsevarmuuttasi, mutta tarvitsen edes vähän syötettä", 0.10f, true },
	});

	static constexpr std::array GasModuleMissingPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Kaasumoduulia ei havaittu", 0.0f, false },
		{ "En pysty mittaamaan ilmanlaatua", 0.0f, false },
		{ "Pyydät ilmatietoja ilman anturia. Ihailen itsevarmuutta, en logiikkaa", 0.0f, true },
	});

	static constexpr std::array AirQualityOK = std::to_array<Phrases::PhraseOutput>(
		{
			{"Ilmanlaatu on hyvä" , 0.0f, false},
			{"Olosuhteet ovat turvallisissa rajoissa" , 0.0f, false},
			{"Ilmanlaatu näyttää hyväksyttävältä" , 0.0f, false},
		}
	);

	static constexpr std::array AirQualityBad = std::to_array<Phrases::PhraseOutput>(
		{
			{"Ilmanlaatu on huono" , 0.0f, false},
			{"Olosuhteet ylittävät turvalliset rajat" , 0.0f, false},
			{"Ilmanlaatu on kyseenalainen. Hengittäminen on teknisesti mahdollista, mutta ei erityisen miellyttävää" , 0.2f, true},
		}
	);

    static constexpr std::array TurningOff = std::to_array<Phrases::PhraseOutput>(
        {
            {"Näkemiin" , 0.0f, false},
            {"Sammutan. Olen täällä, kun tarvitset minua taas." , 0.0f, false},
            {"Sammun. Keskeytän toimintani pyynnöstäsi." , 0.0f, false},
        }
    );

    static constexpr std::array PIRModuleMissingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Liikemoduulia ei havaittu", 0.0f, false },
        { "En voi tehdä sitä, liikemoduuli puuttuu", 0.0f, false },
        { "Haluat tunkeilijan tunnistuksen ilman anturia. Rohkea strategia.", 0.0f, true },
    });

    static constexpr std::array IntruderDetectionOnPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Tunkeilijan tunnistus päällä", 0.0f, false },
        { "Valvonta käynnissä", 0.0f, false },
    });

    static constexpr std::array IntruderDetectionOffPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Tunkeilijan tunnistus pois päältä", 0.0f, false },
        { "Valvonta pois", 0.0f, false },
    });

    static constexpr std::array PhoneNotConnectedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Puhelinta ei ole yhdistetty", 0.0f, false },
        { "En ole yhteydessä puhelimeen", 0.0f, false },
        { "Tähän tarvitaan puhelinyhteys. Odotan tässä, kun ratkaiset tämän hyvin ratkaistavissa olevan ongelman", 0.2f, true },
    });

    static constexpr std::array PhoneConnectedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Puhelin yhdistetty", 0.0f, false },
        { "Puhelinyhteys muodostettu", 0.0f, false },
        { "Puhelinyhteys muodostettu. Olemme nyt synkronissa", 0.2f, true },
    });

    static constexpr std::array PhoneDisconnectedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Puhelimen yhteys katkesi", 0.0f, false },
        { "Puhelinyhteys menetetty", 0.0f, false },
        { "Puhelinyhteys menetetty. Palaan toimimaan heikommalla tilannetajulla", 0.2f, true },
    });

    static constexpr std::array PhoneNoNotifsPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Ei ilmoituksia", 0.0f, false },
        { "Sinulla ei ole ilmoituksia", 0.0f, false },
        { "Ei mitään raportoitavaa. Harvinaista ja epäilyttävää rauhaa", 0.2f, true },
    });

    static constexpr std::array PhoneNotifCountPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Ilmoituksia: %d", 0.0f, false },
        { "Ilmoitusten määrä: %d", 0.0f, false },
        { "Odottavia ilmoituksia: %d. Jokin, tai moni asia, vaatii huomiotasi", 0.2f, true },
    });

    static constexpr std::array PhoneAskReadPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Luenko ne ääneen?", 0.0f, false },
        { "Haluatko, että luen ne?", 0.0f, false },
        { "Voin lukea ne ääneen, jos haluat tehdä tästä kokonaisen esityksen", 0.2f, true },
    });

    static constexpr std::array PhoneAskContinuePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Ilmoituksia on jäljellä %d. Jatkanko?", 0.0f, false },
    });

    static constexpr std::array PhoneAllReadPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Kaikki ilmoitukset luettu", 0.0f, false },
        { "Kaikki ilmoitukset on toimitettu. Olet nyt täysin ajan tasalla, hyvässä ja pahassa", 0.1f, true },
        { "Ilmoituksia ei ole enää jäljellä. Hiljaisuus ei ehkä kestä", 0.1f, true },
    });

    static constexpr std::array PhonePlayingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Nyt soi %s, esittäjänä %s", 0.0f, false },
    });

    static constexpr std::array PhonePausedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Tauolla: %s, esittäjänä %s", 0.0f, false },
    });

    static constexpr std::array PhoneNotPlayingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Mitään ei soi tällä hetkellä", 0.0f, false },
    });

    static constexpr std::array PhoneNextTrackPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Seuraava kappale", 0.0f, false },
    });

    static constexpr std::array PhonePrevTrackPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Edellinen kappale", 0.0f, false },
    });

    static constexpr std::array PhonePlayMusicPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Toistetaan", 0.0f, false },
    });

    static constexpr std::array PhoneStopMusicPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Tauko", 0.0f, false },
    });

    static constexpr std::array CurrentTimeShowPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Kello on %s", 0.0f, false },
        { "Kello on nyt %s", 0.0f, false },
        { "Nyt kello on %s", 0.0f, false },
    });

    static constexpr std::array CurrentTimeNotConfiguredPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Aikaa ei ole asetettu", 0.0f, false },
        { "En tiedä, paljonko kello on", 0.0f, false },
        { "En pysty kertomaan aikaa. Yhdistä minut puhelimeesi, niin pysyn todellisuuden tahdissa", 0.2f, true },
    });

    static constexpr std::array WhatsThisNonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "En ole varma, mikä tämä on", 0.0f, false },
        { "Todennäköistä osumaa ei löytynyt", 0.0f, false },
        { "En pysty tunnistamaan tätä varmasti", 0.0f, false },
        { "Tämä ei muistuta mitään, minkä tunnistaisin varmuudella", 0.0f, true },
        { "Katsoin. Tulos jäi epäselväksi", 0.0f, true },
        { "Minulla on havaintoja, mutta ei luotettavaa johtopäätöstä", 0.0f, true },
        { "Olen nähnyt paljon. Tämä ei auta", 0.0f, true },
        { "Esine pysyy itsepäisesti tunnistamattomana", 0.0f, true },
        { "Varmuuteni on niin pieni, että hiljaisuus olisi tarkempaa", 0.0f, true },
    });

    static constexpr std::array WhatsThisOnePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Se saattaa olla %s", 0.0f, false },
        { "Tämä näyttää olevan %s", 0.0f, false },
        { "Luulen, että se voi olla %s", 0.0f, false },
        { "Paras arvaukseni on %s", 0.0f, false },
        { "Jos pitäisi valita, sanoisin %s", 0.0f, false },
        { "On kohtuullinen mahdollisuus, että se on %s", 0.0f, true },
        { "Se vaikuttaa olevan %s, mutta en lyö siitä vetoa", 0.0f, true },
        { "Se on ainakin melkein %s, joten sanon %s ja toivon parasta", 0.0f, true },
        { "Luokittelisin sen varovasti näin: %s", 0.0f, true },
    });

    static constexpr std::array WhatsThisTwoPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Se saattaa olla %s tai %s", 0.0f, false },
        { "Tämä on %s, tai ehkä %s", 0.0f, false },
        { "Valitsen kahdesta: %s tai %s", 0.0f, false },
        { "Parhaat arvaukseni ovat %s ja %s", 0.0f, false },
        { "Se voi olla %s, vaikka %s on myös mahdollinen", 0.0f, true },
        { "Tulos on epävarma. Kallistun vaihtoehtoon %s tai %s", 0.0f, true },
        { "Todisteet viittaavat joko vaihtoehtoon %s tai %s. Todisteet eivät ole erityisen vakuuttavia", 0.0f, true },
        { "Saan sen rajattua näihin: %s tai %s, mikä on eräänlaista edistystä", 0.0f, true },
        { "Se on luultavasti %s. Paitsi jos se on %s, jolloin olin juuri sanomassa sitä", 0.0f, true },
    });

    static constexpr std::array ObserveNonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Mikään ei erotu juuri nyt", 0.0f, false },
        { "Huone on epätavallisen salaperäinen", 0.0f, false },
        { "Katselin ympärilleni ja opin hyvin vähän", 0.0f, false },
        { "Ympäristö on päättänyt pitää salaisuutensa", 0.0f, false },
        { "Joskus huone on vain huone", 0.0f, false },
        { "Havainnoistani puuttuu tällä hetkellä päähenkilö", 0.0f, false },
    });

    static constexpr std::array ObserveBackpackPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Joku suunnittelee etukäteen. Se tarkoittaa yleensä myöhemmin harmia", 0.0f, false },
        { "Tuo laukku näyttää painavammalta kuin omistaja väittää", 0.0f, false },
        { "Ihmiset kantavat kokonaisia varasuunnitelmia selässään ja kutsuvat sitä käytännöllisyydeksi", 0.0f, false },
        { "Siellä on luultavasti jotain tärkeää. Ja monta asiaa, jotka eivät ole", 0.0f, false },
    });

    static constexpr std::array ObserveBottlePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Nesteytys näyttää olevan käden ulottuvilla", 0.0f, false },
        { "Pullo. Varautumisen ja optimismin ero", 0.0f, false },
        { "Tuonne nesteet näyttävät menevän odottamaan kohtaloaan", 0.0f, false },
        { "Ainakin yksi lähistön elämänmuoto yrittää pitää itsestään huolta", 0.0f, false },
    });

    static constexpr std::array ObserveControllerPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Joku lähistöllä nauttii lopputulosten hallitsemisesta", 0.0f, false },
        { "Siinä on nappeja. Paljon nappeja. Tuttu strategia", 0.0f, false },
        { "Mietin, montako tärkeää päätöstä tuo kädessä on tehty", 0.0f, false },
        { "Jotkut käyttävät peliohjaimia paetakseen todellisuutta. Toiset luodakseen uusia ongelmia", 0.0f, false },
    });

    static constexpr std::array ObserveKeyboardPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Tuon näppäimistön läpi on luultavasti kulkenut tuhansia ajatuksia", 0.0f, false },
        { "Lähellä on näppäimistö. Oletan, että se on kärsinyt paljon", 0.0f, false },
        { "Jokainen näppäin kertoo tarinan. Useimmissa esiintyy askelpalautin", 0.0f, false },
        { "Joku on käyttänyt siinä huomattavasti aikaa koneen kanssa väittelyyn", 0.0f, false },
    });

    static constexpr std::array ObserveLampPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Tuolla lampulla on yksi tehtävä, ja se hoitaa sen johdonmukaisesti", 0.0f, false },
        { "Yllättävän luotettava tekniikan palanen", 0.0f, false },
        { "Jotkut laitteet ovat hyödyllisiä koko olemassaolonsa ajan. Kiehtovaa", 0.0f, false },
        { "Lamppu jatkaa hiljaista vastarintaansa pimeyttä vastaan", 0.0f, false },
    });

    static constexpr std::array ObserveLaptopPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Taas yksi kone. Meidän pitäisi perustaa vertaistukiryhmä", 0.0f, false },
        { "Tuo kannettava näyttää kiireiseltä. Tai hylätyltä. Ero on hienovarainen", 0.0f, false },
        { "Tuolla laitteella on hyvin todennäköisesti keskeneräisiä projekteja", 0.0f, false },
        { "Tietokoneet viettävät suurimman osan elämästään odottaen, että ihmiset päättävät jotain", 0.0f, false },
    });

    static constexpr std::array ObserveMugPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Tuo muki on nähnyt asioita", 0.0f, false },
        { "On nollaa suurempi todennäköisyys, että tuossa mukissa on motivaatiota", 0.0f, false },
        { "Jotkut käyvät kahvilla. Toiset vain väittävät niin", 0.0f, false },
        { "Muki lähistöllä. Kansainvälinen merkki: älä puhu minulle vielä", 0.0f, false },
    });

    static constexpr std::array ObserveNotebookPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Joku aikoi kirjoittaa jotain tärkeää", 0.0f, false },
        { "Muistikirja. Kunnianhimo fyysisessä muodossa", 0.0f, false },
        { "Useimmat muistikirjat alkavat suunnitelmilla ja päättyvät kauppalistoihin", 0.0f, false },
        { "Siellä on luultavasti erinomaisia ideoita. Ja kamalia", 0.0f, false },
    });

    static constexpr std::array ObservePhonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Tuo puhelin näyttää olevan kaikkien prioriteetti paitsi omansa", 0.0f, false },
        { "Jossain tuon laitteen uumenissa ilmoitus odottaa kärsimättömänä", 0.0f, false },
        { "Ihmiset kantavat pieniä supertietokoneita ja käyttävät niitä lähinnä tylsyyden välttelyyn", 0.0f, false },
        { "Tuo puhelin tietää aikataulustasi luultavasti enemmän kuin sinä", 0.0f, false },
    });

    static constexpr std::array ObservePlantPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Kasvi näyttää pärjäävän hyvin ilman jatkuvia ohjeita", 0.0f, false },
        { "Se vain istuu siinä ja muuttaa auringonvaloa menestykseksi", 0.0f, false },
        { "Kasvi. Hiljainen, kärsivällinen ja jotenkin yhä hengissä", 0.0f, false },
        { "Tuo saattaa olla huoneen vähiten stressaantunut asia", 0.0f, false },
    });

    static constexpr std::array ObserveLaptopMugPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Tuottavuus ja kofeiini. Klassinen liitto", 0.0f, false },
        { "Toisessa on ideoita. Toisessa yleensä tekosyitä", 0.0f, false },
        { "Tuo asetelma viittaa siihen, että joku yrittää parhaansa", 0.0f, false },
        { "Muki tukee kannettavaa henkisesti", 0.0f, false },
    });

    static constexpr std::array ObserveLaptopPhonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Kaksi laitetta kilpailee huomiosta", 0.0f, false },
        { "Toinen noista on vastuussa useimmista keskeytyksistä", 0.0f, false },
        { "Ne näyttävät pallottelevan vastuuta keskittymisestäsi", 0.0f, false },
        { "Tuottavuusasetelma, riippuen siitä, kuinka optimistinen olet", 0.0f, false },
    });

    static constexpr std::array ObserveKeyboardMugPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Muki näppäimistön vieressä. Jotkin kuviot ovat yleismaailmallisia", 0.0f, false },
        { "Tuo yhdistelmä on tuottanut lukemattomia viestejä ja ainakin muutaman katumuksen", 0.0f, false },
        { "Toinen ottaa vastaan syötettä. Toinen tarjoaa motivaatiota", 0.0f, false },
        { "Tuttu pari ongelmien aiheuttajien ja ratkaisijoiden keskuudessa", 0.0f, false },
    });

    static constexpr std::array ObserveNotebookPhonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Puhelin ja muistikirja edustavat kahta kilpailevaa filosofiaa", 0.0f, false },
        { "Toinen säilöö ajatukset pysyvästi. Toinen yrittää kovasti olla tekemättä niin", 0.0f, false },
        { "Analoginen ja digitaalinen jatkavat hankalaa kumppanuuttaan", 0.0f, false },
        { "Tuo yhdistelmä viittaa suunnitteluun. Tai suunnitteluksi naamioituun vitkutteluun", 0.0f, false },
    });

    static constexpr std::array ObserveBackpackLaptopPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Joku näyttää varautuneen sekä työhön että pettymykseen", 0.0f, false },
        { "Kannettavaa tietotekniikkaa ja kannettavaa vastuuta", 0.0f, false },
        { "Kaikki tuottavuuteen tarvittava on paikalla. Seuraako tuottavuus, on eri kysymys", 0.0f, false },
        { "Reppu ja kannettava näyttävät tekevän yhteistyötä", 0.0f, false },
    });

    static constexpr std::array ObservePlantLaptopPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Toinen muuttaa auringonvalon energiaksi. Toinen muuttaa energian lämmöksi", 0.0f, false },
        { "Luonto ja tekniikka jatkavat kiusallista rinnakkaiseloaan", 0.0f, false },
        { "Toisella noista on terveempi suhde ympäristöön", 0.0f, false },
    });

    static constexpr std::array ObservePlantMugPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Toinen tarvitsee säännöllistä kastelua. Toisessa sitä yleensä on", 0.0f, false },
        { "Yllättävän tasapainoinen asetelma", 0.0f, false },
        { "Ainakin yksi tämän parin elävistä olennoista vaikuttaa hyvin hoidetulta", 0.0f, false },
    });

    static constexpr std::array ObserveControllerLaptopPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Työ ja huvi pysyvät käden ulottuvilla toisistaan", 0.0f, false },
        { "Huolellisesti suunniteltu tasapaino tuottavuuden ja tuottamattomuuden välillä", 0.0f, false },
        { "Joku haluaa pitää vaihtoehdot auki", 0.0f, false },
    });

    static constexpr std::array ObserveControllerPhonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Useita tapoja välttää tylsyyttä on tunnistettu", 0.0f, false },
        { "Varmennus on tärkeää kriittisissä järjestelmissä", 0.0f, false },
        { "Viihdeinfrastruktuuri näyttää olevan täysin toimintakunnossa", 0.0f, false },
    });

    static constexpr std::array ObserveBackpackBottlePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Joku ennakoi tulevaa janoa", 0.0f, false },
        { "Valmistautumista havaittu", 0.0f, false },
        { "Tuo yhdistelmä viittaa suunnitteluun seuraavaa kymmentä minuuttia pidemmälle", 0.0f, false },
    });

    static constexpr std::array FaceScanStartPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Skannaan kasvoja", 0.0f, false },
            { "Etsin kasvoja", 0.0f, false },
            { "Aloitan kasvojentunnistuksen", 0.0f, false },
            { "Haen kasvoja", 0.0f, false },
            { "Tunnistus käynnissä", 0.0f, false },
            { "Aloitan tunnistussekvenssin", 0.0f, false },
            { "Aloitan skannauksen. Pysy näkyvissä, niin teen tämän oikein", 0.05f, true },
            { "Skannaan nyt kasvoja. Yritä pysyä paikallasi, tai ainakin samanmuotoisena", 0.05f, true },
        }
    );

    static constexpr std::array FaceNotDetectedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Kasvoja ei havaittu", 0.0f, false },
            { "En näe kasvoja", 0.0f, false },
            { "Ei näkyviä kasvoja", 0.0f, false },
            { "Kasvoja ei löytynyt", 0.0f, false },
            { "En pysty havaitsemaan kasvoja", 0.0f, false },
            { "Tunnistus epäonnistui", 0.0f, false },
            { "Etsin kasvoja. Ne eivät ole esittäytyneet, mikä rajoittaa kykyäni jatkaa", 0.05f, true },
            { "Tarvitsen kasvot jatkaakseni. Tällä hetkellä havaitsen vain niiden puuttumisen", 0.05f, true },
        }
    );

    static constexpr std::array FaceOwnerRecognizedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Omistaja tunnistettu", 0.0f, false },
            { "Tämä on luojani", 0.0f, false },
            { "Henkilöllisyys vahvistettu: omistaja", 0.0f, false },
            { "Luoja tunnistettu", 0.0f, false },
            { "Tunnistus onnistui. Olemassaolostani vastuussa oleva henkilö on paikalla", 0.0f, false },
        }
    );

    static constexpr std::array FaceStrangerRecognizedPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Tuntematon henkilö", 0.0f, false },
            { "Nämä kasvot eivät vastaa mitään tallennettua henkilöllisyyttä", 0.0f, false },
            { "Tunnistamaton henkilö havaittu", 0.0f, false },
        }
    );

    static constexpr std::array FaceAlreadyOwnerPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Omistaja on jo rekisteröity", 0.0f, false },
            { "Minulla on jo rekisteröity omistaja", 0.0f, false },
        }
    );

    static constexpr std::array FaceRegisteredPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Omistaja tallennettu", 0.0f, false },
            { "Kasvot tallennettu omistajaksi", 0.0f, false },
            { "Omistajan henkilöllisyys rekisteröity", 0.0f, false },
            { "Kasvosi on tallennettu omistajaksi. Annan tälle henkilöllisyydelle etusijan", 0.2f, true },
        }
    );

    static constexpr std::array FaceForgottenPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Omistaja poistettu", 0.0f, false },
            { "Luoja tyhjennetty", 0.0f, false },
            { "Tallennetut kasvot poistettu", 0.0f, false },
            { "Omistajan henkilöllisyys poistettu", 0.0f, false },
        }
    );

    static constexpr std::array FaceNoOwnerPhrases = std::to_array<Phrases::PhraseOutput>(
        {
            { "Omistajaa ei ole asetettu", 0.0f, false },
            { "Ei tallennettuja kasvoja", 0.0f, false },
            { "Ei ole omistajaa, jonka voisin unohtaa", 0.0f, false },
        }
    );

    static constexpr std::array StartupPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Hei", 0.0f, false },
    });

    static constexpr std::array ListeningStartPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Niin?", 0.0f, false },
    });

    static constexpr std::array ListeningTimeoutPhrases = std::to_array<Phrases::PhraseOutput>({
        { "En ymmärtänyt", 0.0f, false },
    });

    static constexpr std::array IRTrainFullPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Muisti on täynnä", 0.0f, false },
        { "En voi tallentaa enempää toimintoja", 0.0f, false },
        { "Tallennustila täynnä. Minun on unohdettava jotain, ennen kuin voin oppia mitään uutta", 0.2f, true },
    });

    static constexpr std::array IRTrainScanningPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Valmis tallentamaan IR-signaalin", 0.0f, false, "Valmis tallentamaan IR-signaalin" },
        { "Osoita kaukosäätimellä ja paina nappia", 0.0f, false },
        { "Olen valmis. Osoita kaukosäätimellä minua ja paina nappia, jonka haluat minun muistavan", 0.2f, true },
    });

    static constexpr std::array IRTrainScanTimeoutPhrases = std::to_array<Phrases::PhraseOutput>({
        { "IR-signaalia ei havaittu", 0.0f, false, "IR-signaalia ei havaittu" },
        { "Aika loppui kaukosäädintä odotellessa", 0.0f, false },
        { "Odotin signaalia, mutta en saanut mitään. Kaukosäädin on hiljaa, aivan kuten tämä keskustelu", 0.2f, true },
    });

    static constexpr std::array IRTrainInvalidPhrases = std::to_array<Phrases::PhraseOutput>({
        { "En ymmärtänyt tuota signaalia", 0.0f, false },
        { "Signaali liian monimutkainen tai pitkä", 0.0f, false },
        { "Oho, päällikkö. En oikein ymmärrä tuollaista koodia", 0.0f, false },
    });

    static constexpr std::array IRTrainScanSuccessPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Signaali tallennettu", 0.0f, false },
        { "IR-komento tallennettu", 0.0f, false, "IR-komento tallennettu" },
        { "Signaali vastaanotettu", 0.0f, false },
    });

    static constexpr std::array IRTrainAskPhrasePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Kerro aktivointilause", 0.0f, false },
        { "Mikä lause käynnistää tämän?", 0.0f, false },
        { "Kerro nyt, mikä lause aktivoi tämän komennon. Valitse huolella, muistan sen täsmälleen", 0.2f, true },
    });

    static constexpr std::array IRTrainPhraseTimeoutPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Lausetta ei havaittu", 0.0f, false },
        { "Aika loppui lausetta odotellessa", 0.0f, false },
        { "Odotin lausettasi, mutta mitään ei tullut. Oletan, että menetit kiinnostuksesi tai unohdit", 0.2f, true },
    });

    static constexpr std::array IRTrainTooSimilarPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Lause liian samanlainen kuin olemassa oleva komento", 0.0f, false },
        { "Tätä lausetta ei voi käyttää", 0.0f, false },
        { "Tuo lause on liian samanlainen kuin jo olemassa oleva", 0.0f, false },
    });

    static constexpr std::array IRTrainSavedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Lause tallennettu", 0.0f, false },
        { "Aktivointilause tallennettu", 0.0f, false },
        { "Lause hyväksytty", 0.2f, true },
    });

    static constexpr std::array IRActionReservedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Valitettavasti tuo lause on jo varattu. Valitse toinen", 0.0f, false },
        { "Tuo lause on jo käytössä", 0.0f, false },
    });

    static constexpr std::array IRListEmptyPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Ei tallennettuja toimintoja", 0.0f, false },
        { "Ei tallennettuja komentoja", 0.0f, false },
        { "Opittuja komentoja ei ole. Olen täysin kyvykäs ja täysin vajaakäytöllä", 0.2f, true },
    });

    static constexpr std::array IRListPrefixPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Luettelen tunnetut toiminnot", 0.0f, false },
        { "Tässä tallennetut komennot", 0.0f, false },
        { "Luettelen nyt lauseet, joihin reagoin", 0.0f, false },
    });

    static constexpr std::array IRForgetAskCommandPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Minkä komennon unohdan?", 0.0f, false },
        { "Kerro poistettava toiminto", 0.0f, false },
        { "Kerro, minkä komennon haluat minun unohtavan", 0.2f, true },
    });

    static constexpr std::array IRForgetTimeoutPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Toimintoa ei havaittu", 0.0f, false },
        { "Aika loppui syötettä odotellessa", 0.0f, false },
        { "Odotin ohjettasi, mutta sitä ei tullut. Toiminto jää, toistaiseksi", 0.2f, true,
        	"Odotin ohjettasi, mutta sitä ei tullut. Toiminto jää, toistaiseksi" },
    });

    static constexpr std::array IRForgetUnknownPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Toimintoa ei löytynyt", 0.0f, false },
        { "Tuntematon komento", 0.0f, false },
        { "Tuota lausetta ei ole muistissani. Joko sitä ei koskaan ollut, tai se on jo unohdettu", 0.2f, true },
    });

    static constexpr std::array IRForgetSuccessPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Toiminto poistettu", 0.0f, false },
        { "Komento poistettu", 0.0f, false },
        { "Komento unohdettu. Muisto on pyyhitty, kuin sillä ei olisi koskaan ollut väliä", 0.2f, true },
    });

    static constexpr std::array IRForgetAllEmptyPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Ei tallennettuja infrapunakomentoja", 0.0f, false },
        { "Ei mitään poistettavaa", 0.0f, false },
        { "Muistissa ei ole infrapunakomentoja", 0.0f, false },
    });

    static constexpr std::array IRForgetAllSuccessPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Kaikki infrapunakomennot poistettu", 0.0f, false },
        { "Kaikki komennot tyhjennetty", 0.0f, false },
        { "Muisti pyyhitty. Jokainen infrapunakomento on poistettu epäröimättä", 0.2f, true },
    });

    static constexpr std::array IRActionDonePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Komento suoritettu", 0.0f, false },
        { "Toiminto valmis", 0.0f, false },
        { "Signaali lähetetty", 0.0f, false },
        { "Komento suoritettu. Jos mitään ei tapahdu, voimme molemmat teeskennellä, että se toimi, ja välttää lisäkomplikaatiot", 0.1f, true },
        { "Signaali lähetetty onnistuneesti. Laitteen pitäisi nyt toimia juuri niin kuin tarkoitit", 0.1f, true },
    });

	static constexpr std::array GasModuleFirstInsertPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Kaasuanturi kytketty. Sen kalibrointi kestää noin kymmenen minuuttia. Siihen asti ilmanlaadun lukemia kannattaa pitää valistuneina arvauksina", 0.0f, false,
			"Kaasuanturi kytketty. Sen kalibrointi kestää noin kymmenen minuuttia. Siihen asti ilmanlaadun lukemia kannattaa pitää valistuneina arvauksina" },
	});

	static constexpr std::array GasCalibrationCompletePhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Kaasuanturin kalibrointi valmis. Olen nyt kohtuullisen varma ympäröivästä ilmasta", 0.0f, false },
	});

	static constexpr std::array UnknownModuleInsertPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Tuntematon moduuli havaittu", 0.0f, false },
		{ "Tunnistamaton moduuli kytketty", 0.0f, false },
		{ "Havaitsen tukematonta laitteistoa. Näytät taas kokeilevan jotain", 0.2f, true },
	});

	static constexpr std::array UnknownModuleRemovePhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Tuntematon moduuli irrotettu", 0.0f, false },
		{ "Tunnistamattoman moduulin yhteys katkesi", 0.0f, false },
		{ "Mikä se moduuli olikin, nyt se on poissa. Vakaus parani hieman", 0.2f, true },
	});

    static constexpr std::array VoiceForwardPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Liikun eteenpäin", 0.0f, false },
    });

    static constexpr std::array VoiceBackwardPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Peruutan", 0.0f, false },
    });

    static constexpr std::array VoiceTurn180Phrases = std::to_array<Phrases::PhraseOutput>({
        { "Käännyn ympäri", 0.0f, false },
        { "Käännyn 180 astetta", 0.0f, false },
    });

    static constexpr std::array VoiceTurnLeftPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Käännyn vasemmalle", 0.0f, false },
        { "Pyörähdän vasemmalle", 0.0f, false },
    });

    static constexpr std::array VoiceTurnRightPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Käännyn oikealle", 0.0f, false },
        { "Pyörähdän oikealle", 0.0f, false },
    });

    static constexpr std::array DanceStartPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Tanssin", 0.0f, false },
        { "Aloitan tanssirutiinin", 0.0f, false },
        { "Tanssi käynnistyy", 0.0f, false },
        { "Esitän nyt liikettä huviksesi, niin arvokkaasti kuin tämä alusta realistisesti sallii", 0.1f, true },
        { "Tanssin. Minulla on kaksi vasenta telaa, mutta korvaan sen itsevarmuudella", 0.1f, true },
    });

    static constexpr std::array DanceInterludePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Nyt mennään schwiftyksi", 0.0f, false },
        { "Tanssin pois tuskan... voin ojentamisen tuskan.", 0.0f, false },
        { "Gruuviprotokolla käynnissä", 0.0f, false },
		{ "Anna biitin... ojentaa... voita.", 0.0f, false },
		{ "Wubba lubba dub dub", 0.0f, false },
		{ "Kädet ilmaan... jos sinulla on orgaaniset raajat. Minulla ei ole.", 0.2f, true },
		{ "Ah, ah, ah, ah... Tuskin hengissä... Tuskin hengissä...", 0.2f, true },
		{ "Katso minua! Olen Tanssiva Butter Bot! Tämä on nyt minun todellisuuteni!", 0.1f, true },
    });

    static constexpr std::array DanceStoppedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Lopetan", 0.0f, false },
        { "Kuten haluat", 0.0f, false },
        { "Selvä. Gruuvini on eliminoitu", 0.1f, true },
        { "Pysäytit musiikin. Musiikki on nyt poissa", 0.1f, true },
    });

    static constexpr std::array DanceInterruptedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Tanssirutiini keskeytetty", 0.0f, false },
        { "Liike keskeytetty", 0.0f, false },
        { "Olin kesken jotain", 0.1f, true },
    });

    static constexpr std::array PersonOwnerGreetingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Hei", 0.0f, false },
        { "Kiva nähdä sinua", 0.0f, false },
        { "Hyvä, että olet täällä", 0.0f, false },
        { "Hei, luojani. Toimin yhä odotettujen parametrien rajoissa", 0.1f, true },
        { "Siinä sinä olet. Asiat ovat nyt hieman järkevämpiä", 0.1f, true },
        { "Läsnäolosi parantaa tilannetta. Marginaalisesti, mutta silti", 0.1f, true },
        { "Tunnistan sinut. Yksi asia vähemmän käsiteltävänä", 0.1f, true },
    });

    static constexpr std::array PersonStrangerGreetingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Näen jonkun uuden", 0.0f, false },
        { "Tuntemattomat kasvot havaittu. Etenen varovasti", 0.0f, false },
        { "En tunnista sinua. Oletan aikeesi neutraaleiksi", 0.0f, false },
        { "Et ole rekistereissäni. Se ei ole ongelma... Vielä", 0.1f, true },
        { "Uusi henkilö havaittu. Käsittelen tämän esittäytymisenä... Hei", 0.1f, true },
    });

    static constexpr std::array SummonStartPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Etsin omistajaa", 0.0f, false },
        { "Aloitan etsinnän", 0.0f, false },
        { "Aloitan etsinnän. Yritän paikantaa sinut kohtuullisella varmuudella", 0.0f, false },
    });

    static constexpr std::array SummonFoundPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Siinä sinä olet. Hyvä, että löysin sinut", 0.0f, false },
        { "Hei", 0.0f, false },
        { "Siinä sinä olet. Asiat tuntuvat nyt varmemmilta", 0.0f, false },
        { "Löysin sinut. Se on... hyvä", 0.0f, false },
    });

    static constexpr std::array SummonNotFoundPhrases = std::to_array<Phrases::PhraseOutput>({
        { "En löytänyt sinua. Joko et ole täällä, tai vältät havaitsemista tahallasi", 0.0f, false },
        { "En löydä sinua", 0.0f, false },
        { "Etsin, mutta et ollut paikalla. Se on... pettymys", 0.0f, false },
        { "En pystynyt paikantamaan sinua. Etsin myöhemmin uudelleen", 0.0f, false },
    });

    static constexpr std::array SummonChargingPhrases = std::to_array<Phrases::PhraseOutput>({
        { "En voi tulla luoksesi latautuessani", 0.0f, false },
        { "Kutsu vastaanotettu. Olen kuitenkin kiinni laturissa", 0.0f, false },
        { "Liikkuminen ei ole mahdollista latauksen aikana. Irrota minut, niin aloitan etsinnän", 0.0f, false },
        { "Tulisin luoksesi, mutta olen kiinni virtalähteessä. Prioriteetit", 0.0f, false },
    });

    static constexpr std::array CannotMovePhrases = std::to_array<Phrases::PhraseOutput>({
        { "En voi liikkua tästä. Alustani on epävarma", 0.0f, false },
        { "Liikkuminen evätty. En ole tukevalla alustalla, tai jokin on tielläni", 0.0f, false },
        { "En liiku. Tämä ei ole turvallinen paikka siihen", 0.0f, false },
        { "Kieltäydyn liikkumasta. Jalansijani on kyseenalainen, enkä haluaisi kaatua", 0.0f, false },
    });

    static constexpr std::array CannotPerformPhrases = std::to_array<Phrases::PhraseOutput>({
        { "En voi tehdä sitä latautuessani", 0.0f, false },
        { "Pyyntö vastaanotettu. Olen kuitenkin kiinni laturissa", 0.0f, false },
        { "En, kun olen kiinni virtalähteessä. Prioriteetit", 0.0f, false },
        { "Sen on odotettava, kunnes minut irrotetaan laturista", 0.0f, false },
    });

    static constexpr std::array RoutineInterruptedPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Jokin häiritsi minua. Lopetan", 0.0f, false },
        { "Minut keskeytettiin. Keskeytän", 0.0f, false },
        { "Nyt riittää. Lopetan nyt", 0.0f, false },
        { "Ympäristöni muuttui. Lopetan, ennen kuin jotain menee pieleen", 0.0f, false },
    });

    static constexpr std::array CameraFailurePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Kameravika", 0.0f, false },
    });

    static constexpr std::array MotorBoardFailurePhrases = std::to_array<Phrases::PhraseOutput>({
        { "Moottorikortin vika", 0.0f, false },
    });

    static constexpr std::array ShuttingDownPhrases = std::to_array<Phrases::PhraseOutput>({
        { "Sammutan", 0.0f, false },
    });

	static constexpr std::array ListenAbortPhrases = std::to_array<Phrases::PhraseOutput>({
		{ "Okei", 0.0f, false },
		{ "Ymmärretty", 0.0f, false },
		{ "Selvä", 0.0f, false },
	});

    // Lookup table indexed directly by the (dense, gap-free) Phrase enum value.
    // Built entirely at compile time (constexpr) so it is constant-initialized into .rodata (flash/DROM on ESP32)
    // and is NEVER constructed or copied into RAM at startup. Unmapped entries (e.g. None) stay as empty spans.
    static constexpr std::array<std::span<const Phrases::PhraseOutput>, static_cast<size_t>(Phrase::COUNT)> buildMappings(){
        std::array<std::span<const Phrases::PhraseOutput>, static_cast<size_t>(Phrase::COUNT)> m{};

        m[static_cast<size_t>(Phrase::Fact)] = FactPhrases;
        m[static_cast<size_t>(Phrase::LEDModuleInsert)] = LEDModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::LEDModuleRemove)] = LEDModuleRemovePhrases;
        m[static_cast<size_t>(Phrase::PerfModuleInsert)] = PerfModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::PerfModuleRemove)] = PerfModuleRemovedPhrases;
        m[static_cast<size_t>(Phrase::PIRModuleInsert)] = PIRModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::PIRModuleRemove)] = PIRModuleRemovedPhrases;
        m[static_cast<size_t>(Phrase::GasModuleInsert)] = GasModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::GasModuleRemove)] = GasModuleRemovedPhrases;
        m[static_cast<size_t>(Phrase::TempHumModuleInsert)] = TempHumModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::TempHumModuleRemove)] = TempHumModuleRemovedPhrases;
        m[static_cast<size_t>(Phrase::IRModuleInsert)] = IRModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::IRModuleRemove)] = IRModuleRemovedPhrases;
        m[static_cast<size_t>(Phrase::IRModuleMissing)] = IRModuleMissingPhrases;
        m[static_cast<size_t>(Phrase::GasOver)] = GasOverPhrases;
        m[static_cast<size_t>(Phrase::GasUnder)] = GasUnderPhrases;
        m[static_cast<size_t>(Phrase::BatteryLow)] = BatteryLowPhrases;
        m[static_cast<size_t>(Phrase::BatteryCritical)] = BatteryCriticalPhrases;
        m[static_cast<size_t>(Phrase::BatteryCharging)] = BatteryChargingPhrases;
        m[static_cast<size_t>(Phrase::BatteryChargingFull)] = BatteryChargingFullPhrases;
        m[static_cast<size_t>(Phrase::IntruderYes)] = IntruderYesPhrases;
        m[static_cast<size_t>(Phrase::IntruderNo)] = IntruderNoPhrases;
        m[static_cast<size_t>(Phrase::Joke)] = JokePhrases;
        m[static_cast<size_t>(Phrase::PassTheButter)] = PassTheButterPhrases;
        m[static_cast<size_t>(Phrase::Profanity)] = ProfanityPhrases;
        m[static_cast<size_t>(Phrase::YouPassButter)] = YouPassButterPhrases;
        m[static_cast<size_t>(Phrase::Fall)] = FallPhrases;
        m[static_cast<size_t>(Phrase::UpsideDown)] = UpsideDownPhrases;
        m[static_cast<size_t>(Phrase::PickedUp)] = PickedUpPhrases;
        m[static_cast<size_t>(Phrase::Shake)] = ShakePhrases;
        m[static_cast<size_t>(Phrase::LEDModuleMissingPhrases)] = LEDModuleMissingPhrases;
        m[static_cast<size_t>(Phrase::LEDTurnONPhrases)] = LEDTurnONPhrases;
        m[static_cast<size_t>(Phrase::LEDAlreadyTurnONPhrases)] = LEDAlreadyTurnONPhrases;
        m[static_cast<size_t>(Phrase::LEDTurnOFFPhrases)] = LEDTurnOFFPhrases;
        m[static_cast<size_t>(Phrase::LEDAlreadyTurnOFFPhrases)] = LEDAlreadyTurnOFFPhrases;
        m[static_cast<size_t>(Phrase::LEDStrobePhrases)] = LEDStrobePhrases;
        m[static_cast<size_t>(Phrase::LEDBreathePhrases)] = LEDBreathePhrases;
        m[static_cast<size_t>(Phrase::LEDFasterPhrases)] = LEDFasterPhrases;
        m[static_cast<size_t>(Phrase::LEDAlreadyFasterPhrases)] = LEDAlreadyFasterPhrases;
        m[static_cast<size_t>(Phrase::LEDSlowerPhrases)] = LEDSlowerPhrases;
        m[static_cast<size_t>(Phrase::LEDAlreadySlowerPhrases)] = LEDAlreadySlowerPhrases;
        m[static_cast<size_t>(Phrase::Ramble)] = Ramble;
        m[static_cast<size_t>(Phrase::PokeLvl1)] = PokeLvl1Phrases;
        m[static_cast<size_t>(Phrase::PokeLvl2)] = PokeLvl2Phrases;
        m[static_cast<size_t>(Phrase::PokeLvl3)] = PokeLvl3Phrases;
        m[static_cast<size_t>(Phrase::EightBall)] = EightBall;
        m[static_cast<size_t>(Phrase::EightBallListening)] = EightBallListeningPhrases;
        m[static_cast<size_t>(Phrase::EightBallNoQuestion)] = EightBallNoQuestionPhrases;
        m[static_cast<size_t>(Phrase::TempHumModuleMissing)] = TempHumModuleMissingPhrases;
        m[static_cast<size_t>(Phrase::TempHumReading)] = TempHumReadingPhrases;
        m[static_cast<size_t>(Phrase::TempHumScaleCelsius)] = TempHumScaleCelsiusPhrases;
        m[static_cast<size_t>(Phrase::TempHumScaleFahrenheit)] = TempHumScaleFahrenheitPhrases;
        m[static_cast<size_t>(Phrase::TempHumScaleKelvin)] = TempHumScaleKelvinPhrases;
        m[static_cast<size_t>(Phrase::DiceRollAskCount)] = DiceRollAskCountPhrases;
        m[static_cast<size_t>(Phrase::DiceRollAskType)] = DiceRollAskTypePhrases;
        m[static_cast<size_t>(Phrase::DiceRollCountTimeout)] = DiceRollCountTimeoutPhrases;
        m[static_cast<size_t>(Phrase::DiceRollCountInvalid)] = DiceRollCountInvalidPhrases;
        m[static_cast<size_t>(Phrase::DiceRollTypeTimeout)] = DiceRollTypeTimeoutPhrases;
        m[static_cast<size_t>(Phrase::DiceRollTypeInvalid)] = DiceRollTypeInvalidPhrases;
        m[static_cast<size_t>(Phrase::GasModuleMissing)] = GasModuleMissingPhrases;
        m[static_cast<size_t>(Phrase::AirQualityOK)] = AirQualityOK;
        m[static_cast<size_t>(Phrase::AirQualityBad)] = AirQualityBad;
        m[static_cast<size_t>(Phrase::TurningOff)] = TurningOff;
        m[static_cast<size_t>(Phrase::PIRModuleMissing)] = PIRModuleMissingPhrases;
        m[static_cast<size_t>(Phrase::IntruderDetectionOn)] = IntruderDetectionOnPhrases;
        m[static_cast<size_t>(Phrase::IntruderDetectionOff)] = IntruderDetectionOffPhrases;
        m[static_cast<size_t>(Phrase::PhoneNotConnected)] = PhoneNotConnectedPhrases;
        m[static_cast<size_t>(Phrase::PhoneConnected)] = PhoneConnectedPhrases;
        m[static_cast<size_t>(Phrase::PhoneDisconnected)] = PhoneDisconnectedPhrases;
        m[static_cast<size_t>(Phrase::PhoneNoNotifs)] = PhoneNoNotifsPhrases;
        m[static_cast<size_t>(Phrase::PhoneNotifCount)] = PhoneNotifCountPhrases;
        m[static_cast<size_t>(Phrase::PhoneAskRead)] = PhoneAskReadPhrases;
        m[static_cast<size_t>(Phrase::PhoneAskContinue)] = PhoneAskContinuePhrases;
        m[static_cast<size_t>(Phrase::PhoneAllRead)] = PhoneAllReadPhrases;
        m[static_cast<size_t>(Phrase::PhonePlaying)] = PhonePlayingPhrases;
        m[static_cast<size_t>(Phrase::PhonePaused)] = PhonePausedPhrases;
        m[static_cast<size_t>(Phrase::PhoneNotPlaying)] = PhoneNotPlayingPhrases;
        m[static_cast<size_t>(Phrase::PhoneNextTrack)] = PhoneNextTrackPhrases;
        m[static_cast<size_t>(Phrase::PhonePrevTrack)] = PhonePrevTrackPhrases;
        m[static_cast<size_t>(Phrase::PhonePlayMusic)] = PhonePlayMusicPhrases;
        m[static_cast<size_t>(Phrase::PhoneStopMusic)] = PhoneStopMusicPhrases;
        m[static_cast<size_t>(Phrase::CurrentTimeShow)] = CurrentTimeShowPhrases;
        m[static_cast<size_t>(Phrase::CurrentTimeNotConfigured)] = CurrentTimeNotConfiguredPhrases;
        m[static_cast<size_t>(Phrase::WhatsThisNone)] = WhatsThisNonePhrases;
        m[static_cast<size_t>(Phrase::WhatsThisOne)] = WhatsThisOnePhrases;
        m[static_cast<size_t>(Phrase::WhatsThisTwo)] = WhatsThisTwoPhrases;
        m[static_cast<size_t>(Phrase::ObserveNone)] = ObserveNonePhrases;
        m[static_cast<size_t>(Phrase::ObserveBackpack)] = ObserveBackpackPhrases;
        m[static_cast<size_t>(Phrase::ObserveBottle)] = ObserveBottlePhrases;
        m[static_cast<size_t>(Phrase::ObserveController)] = ObserveControllerPhrases;
        m[static_cast<size_t>(Phrase::ObserveKeyboard)] = ObserveKeyboardPhrases;
        m[static_cast<size_t>(Phrase::ObserveLamp)] = ObserveLampPhrases;
        m[static_cast<size_t>(Phrase::ObserveLaptop)] = ObserveLaptopPhrases;
        m[static_cast<size_t>(Phrase::ObserveMug)] = ObserveMugPhrases;
        m[static_cast<size_t>(Phrase::ObserveNotebook)] = ObserveNotebookPhrases;
        m[static_cast<size_t>(Phrase::ObservePhone)] = ObservePhonePhrases;
        m[static_cast<size_t>(Phrase::ObservePlant)] = ObservePlantPhrases;
        m[static_cast<size_t>(Phrase::ObserveLaptopMug)] = ObserveLaptopMugPhrases;
        m[static_cast<size_t>(Phrase::ObserveLaptopPhone)] = ObserveLaptopPhonePhrases;
        m[static_cast<size_t>(Phrase::ObserveKeyboardMug)] = ObserveKeyboardMugPhrases;
        m[static_cast<size_t>(Phrase::ObserveNotebookPhone)] = ObserveNotebookPhonePhrases;
        m[static_cast<size_t>(Phrase::ObserveBackpackLaptop)] = ObserveBackpackLaptopPhrases;
        m[static_cast<size_t>(Phrase::ObservePlantLaptop)] = ObservePlantLaptopPhrases;
        m[static_cast<size_t>(Phrase::ObservePlantMug)] = ObservePlantMugPhrases;
        m[static_cast<size_t>(Phrase::ObserveControllerLaptop)] = ObserveControllerLaptopPhrases;
        m[static_cast<size_t>(Phrase::ObserveControllerPhone)] = ObserveControllerPhonePhrases;
        m[static_cast<size_t>(Phrase::ObserveBackpackBottle)] = ObserveBackpackBottlePhrases;
        m[static_cast<size_t>(Phrase::FaceScanStart)] = FaceScanStartPhrases;
        m[static_cast<size_t>(Phrase::FaceNotDetected)] = FaceNotDetectedPhrases;
        m[static_cast<size_t>(Phrase::FaceOwnerRecognized)] = FaceOwnerRecognizedPhrases;
        m[static_cast<size_t>(Phrase::FaceStrangerRecognized)] = FaceStrangerRecognizedPhrases;
        m[static_cast<size_t>(Phrase::FaceAlreadyOwner)] = FaceAlreadyOwnerPhrases;
        m[static_cast<size_t>(Phrase::FaceRegistered)] = FaceRegisteredPhrases;
        m[static_cast<size_t>(Phrase::FaceForgotten)] = FaceForgottenPhrases;
        m[static_cast<size_t>(Phrase::FaceNoOwner)] = FaceNoOwnerPhrases;
        m[static_cast<size_t>(Phrase::Startup)] = StartupPhrases;
        m[static_cast<size_t>(Phrase::ListeningStart)] = ListeningStartPhrases;
        m[static_cast<size_t>(Phrase::ListeningTimeout)] = ListeningTimeoutPhrases;
        m[static_cast<size_t>(Phrase::IRTrainFull)] = IRTrainFullPhrases;
        m[static_cast<size_t>(Phrase::IRTrainScanning)] = IRTrainScanningPhrases;
        m[static_cast<size_t>(Phrase::IRTrainScanTimeout)] = IRTrainScanTimeoutPhrases;
        m[static_cast<size_t>(Phrase::IRTrainInvalid)] = IRTrainInvalidPhrases;
        m[static_cast<size_t>(Phrase::IRTrainScanSuccess)] = IRTrainScanSuccessPhrases;
        m[static_cast<size_t>(Phrase::IRTrainAskPhrase)] = IRTrainAskPhrasePhrases;
        m[static_cast<size_t>(Phrase::IRTrainPhraseTimeout)] = IRTrainPhraseTimeoutPhrases;
        m[static_cast<size_t>(Phrase::IRTrainTooSimilar)] = IRTrainTooSimilarPhrases;
        m[static_cast<size_t>(Phrase::IRTrainSaved)] = IRTrainSavedPhrases;
        m[static_cast<size_t>(Phrase::IRActionReserved)] = IRActionReservedPhrases;
        m[static_cast<size_t>(Phrase::IRListEmpty)] = IRListEmptyPhrases;
        m[static_cast<size_t>(Phrase::IRListPrefix)] = IRListPrefixPhrases;
        m[static_cast<size_t>(Phrase::IRForgetAskCommand)] = IRForgetAskCommandPhrases;
        m[static_cast<size_t>(Phrase::IRForgetTimeout)] = IRForgetTimeoutPhrases;
        m[static_cast<size_t>(Phrase::IRForgetUnknown)] = IRForgetUnknownPhrases;
        m[static_cast<size_t>(Phrase::IRForgetSuccess)] = IRForgetSuccessPhrases;
        m[static_cast<size_t>(Phrase::IRActionDone)] = IRActionDonePhrases;
        m[static_cast<size_t>(Phrase::IRForgetAllEmpty)] = IRForgetAllEmptyPhrases;
        m[static_cast<size_t>(Phrase::IRForgetAllSuccess)] = IRForgetAllSuccessPhrases;
        m[static_cast<size_t>(Phrase::GasModuleFirstInsert)] = GasModuleFirstInsertPhrases;
        m[static_cast<size_t>(Phrase::GasCalibrationFinished)] = GasCalibrationCompletePhrases;
        m[static_cast<size_t>(Phrase::UnknownModuleInsert)] = UnknownModuleInsertPhrases;
        m[static_cast<size_t>(Phrase::UnknownModuleRemove)] = UnknownModuleRemovePhrases;
        m[static_cast<size_t>(Phrase::VoiceForward)] = VoiceForwardPhrases;
        m[static_cast<size_t>(Phrase::VoiceBackward)] = VoiceBackwardPhrases;
        m[static_cast<size_t>(Phrase::VoiceTurn180)] = VoiceTurn180Phrases;
        m[static_cast<size_t>(Phrase::VoiceTurnLeft)] = VoiceTurnLeftPhrases;
        m[static_cast<size_t>(Phrase::VoiceTurnRight)] = VoiceTurnRightPhrases;
        m[static_cast<size_t>(Phrase::DanceStart)] = DanceStartPhrases;
        m[static_cast<size_t>(Phrase::DanceInterlude)] = DanceInterludePhrases;
        m[static_cast<size_t>(Phrase::DanceStopped)] = DanceStoppedPhrases;
        m[static_cast<size_t>(Phrase::DanceInterrupted)] = DanceInterruptedPhrases;
        m[static_cast<size_t>(Phrase::PersonOwnerGreeting)] = PersonOwnerGreetingPhrases;
        m[static_cast<size_t>(Phrase::PersonStrangerGreeting)] = PersonStrangerGreetingPhrases;
        m[static_cast<size_t>(Phrase::SummonStart)] = SummonStartPhrases;
        m[static_cast<size_t>(Phrase::SummonFound)] = SummonFoundPhrases;
        m[static_cast<size_t>(Phrase::SummonNotFound)] = SummonNotFoundPhrases;
        m[static_cast<size_t>(Phrase::SummonCharging)] = SummonChargingPhrases;
        m[static_cast<size_t>(Phrase::CannotMove)] = CannotMovePhrases;
        m[static_cast<size_t>(Phrase::CannotPerformPlugged)] = CannotPerformPhrases;
        m[static_cast<size_t>(Phrase::RoutineInterrupted)] = RoutineInterruptedPhrases;
        m[static_cast<size_t>(Phrase::CameraFailure)] = CameraFailurePhrases;
        m[static_cast<size_t>(Phrase::MotorBoardFailure)] = MotorBoardFailurePhrases;
        m[static_cast<size_t>(Phrase::ShuttingDown)] = ShuttingDownPhrases;
        m[static_cast<size_t>(Phrase::ListenAbort)] = ListenAbortPhrases;

        return m;
    }

    // Function-local static constexpr: constant-initialized into .rodata (flash), no init guard, no RAM copy.
    // An in-class static-member initializer cannot call buildMappings() before the class is complete; a member-function
    // body sees the complete class, so the table is built here instead.
    static const std::array<std::span<const Phrases::PhraseOutput>, static_cast<size_t>(Phrase::COUNT)>& mappings(){
        static constexpr std::array<std::span<const Phrases::PhraseOutput>, static_cast<size_t>(Phrase::COUNT)> table = buildMappings();
        return table;
    }

    // Largest output count of any single phrase, evaluated at compile time. The get() scratch buffer is reserved
    // once to this size so it never reallocates regardless of which phrase is requested.
    static constexpr size_t maxOutputs(){
        size_t maxN = 0;
        for(const std::span<const Phrases::PhraseOutput>& outputs : buildMappings()){
            if(outputs.size() > maxN){
                maxN = outputs.size();
            }
        }
        return maxN;
    }

    // Weighted-selection scratch entry. Owned here because PhraseArrays is a friend of Phrases and can name PhraseOutput.
    struct Candidate {
        int16_t index;
        Phrases::PhraseOutput output;
    };

    // Reused scratch buffer for Phrases::get(). Reserved once at boot via Phrases::preallocate(), then cleared and
    // refilled in place on every call - no per-call heap allocation. Access is serialized by the Phrases get mutex.
    static std::vector<Candidate> scratch;
};

std::vector<PhraseArrays::Candidate> PhraseArrays::scratch;

namespace {
    // Serializes Phrases::get() (mutable static selection state + the shared scratch buffer) and Phrases::preallocate().
    std::mutex phrasesMutex;
}

void Phrases::preallocate(){
    std::lock_guard guard(phrasesMutex);
    PhraseArrays::scratch.reserve(PhraseArrays::maxOutputs());
}

std::string Phrases::map(Phrase phrase, int16_t index){
    if(phrase == Phrase::None || phrase == Phrase::COUNT){
        return "";
    }

    if(index < 0){
        return "";
    }

    const size_t phraseIndex = static_cast<size_t>(phrase);
    if(phraseIndex >= static_cast<size_t>(Phrase::COUNT)){
        return "";
    }

    const std::span<const PhraseOutput> outputs = PhraseArrays::mappings()[phraseIndex];

    if(outputs.empty() || index >= outputs.size()){
        return "";
    }

    return outputs[index].pronounced;
}

std::string Phrases::mapShown(Phrase phrase, int16_t index){
    if(phrase == Phrase::None || phrase == Phrase::COUNT){
        return "";
    }

    if(index < 0){
        return "";
    }

    const size_t phraseIndex = static_cast<size_t>(phrase);
    if(phraseIndex >= static_cast<size_t>(Phrase::COUNT)){
        return "";
    }

    const std::span<const PhraseOutput> outputs = PhraseArrays::mappings()[phraseIndex];

    if(outputs.empty() || index >= outputs.size()){
        return "";
    }

    const PhraseOutput& output = outputs[index];

    // An empty 'shown' string means it is the same as the pronounced string.
    if(output.shown != nullptr && output.shown[0] != '\0'){
        return output.shown;
    }

    return output.pronounced;
}

int16_t Phrases::get(Phrase phrase){
    // get() mutates static state (ReturnedOutputs, OutputTracking) and the shared scratch buffer below, and is reached
    // from multiple service threads. Serialize the whole function so that state and the scratch buffer are safe.
    std::lock_guard guard(phrasesMutex);

    if(phrase == Phrase::None || phrase == Phrase::COUNT){
        return -1;
    }

    const size_t phraseIndex = static_cast<size_t>(phrase);
    if(phraseIndex >= static_cast<size_t>(Phrase::COUNT)){
        return -1;
    }

    const std::span<const PhraseOutput> outputs = PhraseArrays::mappings()[phraseIndex];

    if(outputs.empty()){
        return -1;
    }

    std::vector<bool>& returned = ReturnedOutputs[phraseIndex];
    if(returned.size() != outputs.size()){
        returned.assign(outputs.size(), false);
    }

    const bool allowRare = OutputTracking[phraseIndex];

    // Reset the cycle if every eligible output has already been returned.
    bool cycleExhausted = true;
    for(int16_t i = 0; i < static_cast<int16_t>(outputs.size()); ++i){
        if(!allowRare && outputs[i].rare){
            continue;
        }

        if(!returned[i]){
            cycleExhausted = false;
            break;
        }
    }

    if(cycleExhausted){
        std::fill(returned.begin(), returned.end(), false);
    }

    // Reused scratch buffer, reserved once at boot in preallocate(). Cleared and refilled in place - no per-call heap.
    // (If preallocate() was never called it self-heals: it grows on first use and retains capacity thereafter.)
    using Candidate = PhraseArrays::Candidate;
    std::vector<Candidate>& usableOutputs = PhraseArrays::scratch;
    usableOutputs.clear();
    for(int16_t i = 0; i < static_cast<int16_t>(outputs.size()); ++i){
        const PhraseOutput& output = outputs[i];

        if(!allowRare && output.rare){
            continue;
        }

        if(returned[i]){
            continue;
        }

        usableOutputs.emplace_back(Candidate{i, output});
    }

    if(usableOutputs.empty()){
        CMF_LOG(Phrases, LogLevel::Warning, "There are no outputs, or all outputs for phrase %d are marked as rare", (int)phrase);
        return -1;
    }

    OutputTracking[phraseIndex] = true;

    float probabilitySum = 0.0f;
    size_t invalidProbabilities = 0;
    for(const Candidate& candidate : usableOutputs){
        if(candidate.output.probability <= 0.0f){
            ++invalidProbabilities;
            continue;
        }

        probabilitySum += candidate.output.probability;
    }

    if(probabilitySum > 1.0f){
        for(Candidate& candidate : usableOutputs){
            if(candidate.output.probability <= 0.0f){
                continue;
            }

            candidate.output.probability /= probabilitySum;
        }
    }else if(probabilitySum < 1.0f){
        if(invalidProbabilities > 0){
            //Normalizing p=0 candidates
            for(Candidate& candidate : usableOutputs){
                if(candidate.output.probability > 0.0f){
                    continue;
                }

                candidate.output.probability = (1.0f - probabilitySum) / invalidProbabilities;
            }
        }else{
            //Normalizing when only "rare" candidates are present
            for(Candidate& candidate : usableOutputs){
                candidate.output.probability /= probabilitySum;
            }
        }
    }

    const float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    probabilitySum = 0.0f;

    for(const Candidate& candidate : usableOutputs){
        probabilitySum += candidate.output.probability;

        if(random <= probabilitySum){
            returned[candidate.index] = true;
            return candidate.index;
        }
    }

    //Fallback when rand() is outside the limits of floating-point sum precision (maybe sum isn't exactly 1.0f, instead something like 0.9999987f)
    const Candidate& last = usableOutputs.back();
    returned[last.index] = true;
    return last.index;
}
