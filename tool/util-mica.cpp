/*
  ==============================================================================

  Copyright 2007-2013, 2017 William Andrew Burnson
  Copyright 2013-2016 Robert Taub

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  ==============================================================================
*/

#define PRIM_COMPILE_INLINE

//Include prim and bring in its namespace.
#define PRIM_WITH_DIRECTORY
#include "prim.h"

//Explicit global import of prim
namespace
{
  using namespace PRIM_NAMESPACE;
}

/**Manages language codes for both natural and programming languages. Examples
of accepted input formats are: en, en-GB, en-cpp, en-GB-cpp. These correspond
respectively to English, English (UK), C++ with English identifiers,
C++ with English (UK) identifiers. The order is always natural language,
country, programming language. Additionally, the languages are always lowercase
and the countries uppercase. The natural language code is an ISO 639-1 code, the
country is an ISO 3166-2 code, and the programming language code is, in most
cases, the canonical filename extension for source code in that language. The
data for localization can be found in the Localization directory. Language codes
are always case sensitive and thus the canonical form is the only accepted form
for each language. Any combination is legally acceptable even though
combinations can be constructed that do not really exist.*/
class LocalizationData
{
  //Stores tables of language and country codes.
  Tree<String> Languages, Countries, ProgrammingLanguages;

  public:

  static String ISO639_1CodesData()
  {
    String s;
    s << "aa\tAfar\n";
    s << "ab\tAbkhazian\n";
    s << "af\tAfrikaans\n";
    s << "am\tAmharic\n";
    s << "ar\tArabic\n";
    s << "as\tAssamese\n";
    s << "ay\tAymara\n";
    s << "az\tAzerbaijani\n";
    s << "ba\tBashkir\n";
    s << "be\tByelorussian\n";
    s << "bg\tBulgarian\n";
    s << "bh\tBihari\n";
    s << "bi\tBislama\n";
    s << "bn\tBengali\n";
    s << "bo\tTibetan\n";
    s << "br\tBreton\n";
    s << "ca\tCatalan\n";
    s << "co\tCorsican\n";
    s << "cs\tCzech\n";
    s << "cy\tWelch\n";
    s << "da\tDanish\n";
    s << "de\tGerman\n";
    s << "dz\tBhutani\n";
    s << "el\tGreek\n";
    s << "en\tEnglish\n";
    s << "eo\tEsperanto\n";
    s << "es\tSpanish\n";
    s << "et\tEstonian\n";
    s << "eu\tBasque\n";
    s << "fa\tPersian\n";
    s << "fi\tFinnish\n";
    s << "fj\tFiji\n";
    s << "fo\tFaeroese\n";
    s << "fr\tFrench\n";
    s << "fy\tFrisian\n";
    s << "ga\tIrish\n";
    s << "gd\tScots Gaelic\n";
    s << "gl\tGalician\n";
    s << "gn\tGuarani\n";
    s << "gu\tGujarati\n";
    s << "ha\tHausa\n";
    s << "hi\tHindi\n";
    s << "he\tHebrew\n";
    s << "hr\tCroatian\n";
    s << "hu\tHungarian\n";
    s << "hy\tArmenian\n";
    s << "ia\tInterlingua\n";
    s << "id\tIndonesian\n";
    s << "ie\tInterlingue\n";
    s << "ik\tInupiak\n";
    s << "in\tformer Indonesian\n";
    s << "is\tIcelandic\n";
    s << "it\tItalian\n";
    s << "iu\tInuktitut (Eskimo)\n";
    s << "iw\tformer Hebrew\n";
    s << "ja\tJapanese\n";
    s << "ji\tformer Yiddish\n";
    s << "jw\tJavanese\n";
    s << "ka\tGeorgian\n";
    s << "kk\tKazakh\n";
    s << "kl\tGreenlandic\n";
    s << "km\tCambodian\n";
    s << "kn\tKannada\n";
    s << "ko\tKorean\n";
    s << "ks\tKashmiri\n";
    s << "ku\tKurdish\n";
    s << "ky\tKirghiz\n";
    s << "la\tLatin\n";
    s << "ln\tLingala\n";
    s << "lo\tLaothian\n";
    s << "lt\tLithuanian\n";
    s << "lv\tLatvian, Lettish\n";
    s << "mg\tMalagasy\n";
    s << "mi\tMaori\n";
    s << "mk\tMacedonian\n";
    s << "ml\tMalayalam\n";
    s << "mn\tMongolian\n";
    s << "mo\tMoldavian\n";
    s << "mr\tMarathi\n";
    s << "ms\tMalay\n";
    s << "mt\tMaltese\n";
    s << "my\tBurmese\n";
    s << "na\tNauru\n";
    s << "ne\tNepali\n";
    s << "nl\tDutch\n";
    s << "no\tNorwegian\n";
    s << "oc\tOccitan\n";
    s << "om\t(Afan) Oromo\n";
    s << "or\tOriya\n";
    s << "pa\tPunjabi\n";
    s << "pl\tPolish\n";
    s << "ps\tPashto, Pushto\n";
    s << "pt\tPortuguese\n";
    s << "qu\tQuechua\n";
    s << "rm\tRhaeto-Romance\n";
    s << "rn\tKirundi\n";
    s << "ro\tRomanian\n";
    s << "ru\tRussian\n";
    s << "rw\tKinyarwanda\n";
    s << "sa\tSanskrit\n";
    s << "sd\tSindhi\n";
    s << "sg\tSangro\n";
    s << "sh\tSerbo-Croatian\n";
    s << "si\tSinghalese\n";
    s << "sk\tSlovak\n";
    s << "sl\tSlovenian\n";
    s << "sm\tSamoan\n";
    s << "sn\tShona\n";
    s << "so\tSomali\n";
    s << "sq\tAlbanian\n";
    s << "sr\tSerbian\n";
    s << "ss\tSiswati\n";
    s << "st\tSesotho\n";
    s << "su\tSudanese\n";
    s << "sv\tSwedish\n";
    s << "sw\tSwahili\n";
    s << "ta\tTamil\n";
    s << "te\tTegulu\n";
    s << "tg\tTajik\n";
    s << "th\tThai\n";
    s << "ti\tTigrinya\n";
    s << "tk\tTurkmen\n";
    s << "tl\tTagalog\n";
    s << "tn\tSetswana\n";
    s << "to\tTonga\n";
    s << "tr\tTurkish\n";
    s << "ts\tTsonga\n";
    s << "tt\tTatar\n";
    s << "tw\tTwi\n";
    s << "ug\tUigur\n";
    s << "uk\tUkrainian\n";
    s << "ur\tUrdu\n";
    s << "uz\tUzbek\n";
    s << "vi\tVietnamese\n";
    s << "vo\tVolapuk\n";
    s << "wo\tWolof\n";
    s << "xh\tXhosa\n";
    s << "yi\tYiddish\n";
    s << "yo\tYoruba\n";
    s << "za\tZhuang\n";
    s << "zh\tChinese\n";
    s << "zu\tZulu\n";
    return s;
  }

  static String ISO3166_2CodesData()
  {
    String s;
    s << "AD\tAndorra\n";
    s << "AE\tUnited Arab Emirates\n";
    s << "AF\tAfghanistan\n";
    s << "AG\tAntigua and Barbuda\n";
    s << "AI\tAnguilla\n";
    s << "AL\tAlbania\n";
    s << "AM\tArmenia\n";
    s << "AO\tAngola\n";
    s << "AQ\tAntarctica\n";
    s << "AR\tArgentina\n";
    s << "AS\tAmerican Samoa\n";
    s << "AT\tAustria\n";
    s << "AU\tAustralia\n";
    s << "AW\tAruba\n";
    s << "AX\tÅland Islands\n";
    s << "AZ\tAzerbaijan\n";
    s << "BA\tBosnia and Herzegovina\n";
    s << "BB\tBarbados\n";
    s << "BD\tBangladesh\n";
    s << "BE\tBelgium\n";
    s << "BF\tBurkina Faso\n";
    s << "BG\tBulgaria\n";
    s << "BH\tBahrain\n";
    s << "BI\tBurundi\n";
    s << "BJ\tBenin\n";
    s << "BL\tSaint Barthélemy\n";
    s << "BM\tBermuda\n";
    s << "BN\tBrunei Darussalam\n";
    s << "BO\tBolivia, Plurinational State of\n";
    s << "BQ\tBonaire, Sint Eustatius and Saba\n";
    s << "BR\tBrazil\n";
    s << "BS\tBahamas\n";
    s << "BT\tBhutan\n";
    s << "BV\tBouvet Island\n";
    s << "BW\tBotswana\n";
    s << "BY\tBelarus\n";
    s << "BZ\tBelize\n";
    s << "CA\tCanada\n";
    s << "CC\tCocos (Keeling) Islands\n";
    s << "CD\tCongo, the Democratic Republic of the\n";
    s << "CF\tCentral African Republic\n";
    s << "CG\tCongo\n";
    s << "CH\tSwitzerland\n";
    s << "CI\tCôte d'Ivoire\n";
    s << "CK\tCook Islands\n";
    s << "CL\tChile\n";
    s << "CM\tCameroon\n";
    s << "CN\tChina\n";
    s << "CO\tColombia\n";
    s << "CR\tCosta Rica\n";
    s << "CU\tCuba\n";
    s << "CV\tCape Verde\n";
    s << "CW\tCuraçao\n";
    s << "CX\tChristmas Island\n";
    s << "CY\tCyprus\n";
    s << "CZ\tCzech Republic\n";
    s << "DE\tGermany\n";
    s << "DJ\tDjibouti\n";
    s << "DK\tDenmark\n";
    s << "DM\tDominica\n";
    s << "DO\tDominican Republic\n";
    s << "DZ\tAlgeria\n";
    s << "EC\tEcuador\n";
    s << "EE\tEstonia\n";
    s << "EG\tEgypt\n";
    s << "EH\tWestern Sahara\n";
    s << "ER\tEritrea\n";
    s << "ES\tSpain\n";
    s << "ET\tEthiopia\n";
    s << "FI\tFinland\n";
    s << "FJ\tFiji\n";
    s << "FK\tFalkland Islands (Malvinas)\n";
    s << "FM\tMicronesia, Federated States of\n";
    s << "FO\tFaroe Islands\n";
    s << "FR\tFrance\n";
    s << "GA\tGabon\n";
    s << "GB\tUnited Kingdom\n";
    s << "GD\tGrenada\n";
    s << "GE\tGeorgia\n";
    s << "GF\tFrench Guiana\n";
    s << "GG\tGuernsey\n";
    s << "GH\tGhana\n";
    s << "GI\tGibraltar\n";
    s << "GL\tGreenland\n";
    s << "GM\tGambia\n";
    s << "GN\tGuinea\n";
    s << "GP\tGuadeloupe\n";
    s << "GQ\tEquatorial Guinea\n";
    s << "GR\tGreece\n";
    s << "GS\tSouth Georgia and the South Sandwich Islands\n";
    s << "GT\tGuatemala\n";
    s << "GU\tGuam\n";
    s << "GW\tGuinea-Bissau\n";
    s << "GY\tGuyana\n";
    s << "HK\tHong Kong\n";
    s << "HM\tHeard Island and McDonald Islands\n";
    s << "HN\tHonduras\n";
    s << "HR\tCroatia\n";
    s << "HT\tHaiti\n";
    s << "HU\tHungary\n";
    s << "ID\tIndonesia\n";
    s << "IE\tIreland\n";
    s << "IL\tIsrael\n";
    s << "IM\tIsle of Man\n";
    s << "IN\tIndia\n";
    s << "IO\tBritish Indian Ocean Territory\n";
    s << "IQ\tIraq\n";
    s << "IR\tIran, Islamic Republic of\n";
    s << "IS\tIceland\n";
    s << "IT\tItaly\n";
    s << "JE\tJersey\n";
    s << "JM\tJamaica\n";
    s << "JO\tJordan\n";
    s << "JP\tJapan\n";
    s << "KE\tKenya\n";
    s << "KG\tKyrgyzstan\n";
    s << "KH\tCambodia\n";
    s << "KI\tKiribati\n";
    s << "KM\tComoros\n";
    s << "KN\tSaint Kitts and Nevis\n";
    s << "KP\tKorea, Democratic People's Republic of\n";
    s << "KR\tKorea, Republic of\n";
    s << "KW\tKuwait\n";
    s << "KY\tCayman Islands\n";
    s << "KZ\tKazakhstan\n";
    s << "LA\tLao People's Democratic Republic\n";
    s << "LB\tLebanon\n";
    s << "LC\tSaint Lucia\n";
    s << "LI\tLiechtenstein\n";
    s << "LK\tSri Lanka\n";
    s << "LR\tLiberia\n";
    s << "LS\tLesotho\n";
    s << "LT\tLithuania\n";
    s << "LU\tLuxembourg\n";
    s << "LV\tLatvia\n";
    s << "LY\tLibya\n";
    s << "MA\tMorocco\n";
    s << "MC\tMonaco\n";
    s << "MD\tMoldova, Republic of\n";
    s << "ME\tMontenegro\n";
    s << "MF\tSaint Martin (French part)\n";
    s << "MG\tMadagascar\n";
    s << "MH\tMarshall Islands\n";
    s << "MK\tMacedonia, the former Yugoslav Republic of\n";
    s << "ML\tMali\n";
    s << "MM\tMyanmar\n";
    s << "MN\tMongolia\n";
    s << "MO\tMacao\n";
    s << "MP\tNorthern Mariana Islands\n";
    s << "MQ\tMartinique\n";
    s << "MR\tMauritania\n";
    s << "MS\tMontserrat\n";
    s << "MT\tMalta\n";
    s << "MU\tMauritius\n";
    s << "MV\tMaldives\n";
    s << "MW\tMalawi\n";
    s << "MX\tMexico\n";
    s << "MY\tMalaysia\n";
    s << "MZ\tMozambique\n";
    s << "NA\tNamibia\n";
    s << "NC\tNew Caledonia\n";
    s << "NE\tNiger\n";
    s << "NF\tNorfolk Island\n";
    s << "NG\tNigeria\n";
    s << "NI\tNicaragua\n";
    s << "NL\tNetherlands\n";
    s << "NO\tNorway\n";
    s << "NP\tNepal\n";
    s << "NR\tNauru\n";
    s << "NU\tNiue\n";
    s << "NZ\tNew Zealand\n";
    s << "OM\tOman\n";
    s << "PA\tPanama\n";
    s << "PE\tPeru\n";
    s << "PF\tFrench Polynesia\n";
    s << "PG\tPapua New Guinea\n";
    s << "PH\tPhilippines\n";
    s << "PK\tPakistan\n";
    s << "PL\tPoland\n";
    s << "PM\tSaint Pierre and Miquelon\n";
    s << "PN\tPitcairn\n";
    s << "PR\tPuerto Rico\n";
    s << "PS\tPalestine, State of\n";
    s << "PT\tPortugal\n";
    s << "PW\tPalau\n";
    s << "PY\tParaguay\n";
    s << "QA\tQatar\n";
    s << "RE\tRéunion\n";
    s << "RO\tRomania\n";
    s << "RS\tSerbia\n";
    s << "RU\tRussian Federation\n";
    s << "RW\tRwanda\n";
    s << "SA\tSaudi Arabia\n";
    s << "SB\tSolomon Islands\n";
    s << "SC\tSeychelles\n";
    s << "SD\tSudan\n";
    s << "SE\tSweden\n";
    s << "SG\tSingapore\n";
    s << "SH\tSaint Helena, Ascension and Tristan da Cunha\n";
    s << "SI\tSlovenia\n";
    s << "SJ\tSvalbard and Jan Mayen\n";
    s << "SK\tSlovakia\n";
    s << "SL\tSierra Leone\n";
    s << "SM\tSan Marino\n";
    s << "SN\tSenegal\n";
    s << "SO\tSomalia\n";
    s << "SR\tSuriname\n";
    s << "SS\tSouth Sudan\n";
    s << "ST\tSao Tome and Principe\n";
    s << "SV\tEl Salvador\n";
    s << "SX\tSint Maarten (Dutch part)\n";
    s << "SY\tSyrian Arab Republic\n";
    s << "SZ\tSwaziland\n";
    s << "TC\tTurks and Caicos Islands\n";
    s << "TD\tChad\n";
    s << "TF\tFrench Southern Territories\n";
    s << "TG\tTogo\n";
    s << "TH\tThailand\n";
    s << "TJ\tTajikistan\n";
    s << "TK\tTokelau\n";
    s << "TL\tTimor-Leste\n";
    s << "TM\tTurkmenistan\n";
    s << "TN\tTunisia\n";
    s << "TO\tTonga\n";
    s << "TR\tTurkey\n";
    s << "TT\tTrinidad and Tobago\n";
    s << "TV\tTuvalu\n";
    s << "TW\tTaiwan, Province of China\n";
    s << "TZ\tTanzania, United Republic of\n";
    s << "UA\tUkraine\n";
    s << "UG\tUganda\n";
    s << "UM\tUnited States Minor Outlying Islands\n";
    s << "US\tUnited States\n";
    s << "UY\tUruguay\n";
    s << "UZ\tUzbekistan\n";
    s << "VA\tHoly See (Vatican City State)\n";
    s << "VC\tSaint Vincent and the Grenadines\n";
    s << "VE\tVenezuela, Bolivarian Republic of\n";
    s << "VG\tVirgin Islands, British\n";
    s << "VI\tVirgin Islands, U.S.\n";
    s << "VN\tViet Nam\n";
    s << "VU\tVanuatu\n";
    s << "WF\tWallis and Futuna\n";
    s << "WS\tSamoa\n";
    s << "YE\tYemen\n";
    s << "YT\tMayotte\n";
    s << "ZA\tSouth Africa\n";
    s << "ZM\tZambia\n";
    s << "ZW\tZimbabwe\n";
    return s;
  }

  static String ProgrammingLanguagesData()
  {
    String s;
    s << "ada\tAda\n";
    s << "c\tC\n";
    s << "cpp\tC++\n";
    s << "cs\tC#\n";
    s << "d\tD\n";
    s << "es\tECMAScript\n";
    s << "for\tFortran\n";
    s << "hs\tHaskell\n";
    s << "java\tJava\n";
    s << "js\tJavaScript\n";
    s << "lisp\tLisp\n";
    s << "lua\tLua\n";
    s << "m\tMathematica\n";
    s << "mm\tObjective-C++\n";
    s << "pas\tPascal\n";
    s << "php\tPHP\n";
    s << "pl\tPerl\n";
    s << "py\tPython\n";
    s << "rb\tRuby\n";
    s << "sal\tCommon Music\n";
    s << "sc\tSuperCollider\n";
    s << "scala\tScala\n";
    s << "scm\tScheme\n";
    s << "st\tSmalltalk\n";
    s << "vb\tVisual Basic\n";
    s << "xml\tXML\n";
    return s;
  }

  ///Attempts to import the data from the localization files.
  bool Import()
  {
    //Attempt to read in the language data.
    String LanguageCodeData = ISO639_1CodesData();
    String CountryCodeData = ISO3166_2CodesData();
    String ProgrammingLanguageData = ProgrammingLanguagesData();

    //Normalize line endings.
    LanguageCodeData.LineEndingsToLF();
    CountryCodeData.LineEndingsToLF();
    ProgrammingLanguageData.LineEndingsToLF();

    //Get lists for each data file.
    List<String> LanguageList = LanguageCodeData.Tokenize("\n", true),
      CountryList = CountryCodeData.Tokenize("\n", true),
      ProgrammingLanguageList = ProgrammingLanguageData.Tokenize("\n", true);

    //Create tables for each list.
    for(count i = 0; i < LanguageList.n(); i++)
    {
      List<String> x = LanguageList[i].Tokenize("\t");
      if(x.n() != 2)
      {
        C::Out() << "Error while parsing: " << LanguageList[i];
        return false;
      }
      Languages[x[0]] = x[1];
    }

    for(count i = 0; i < CountryList.n(); i++)
    {
      List<String> x = CountryList[i].Tokenize("\t");
      if(x.n() != 2)
      {
        C::Out() << "Error while parsing: " << CountryList[i];
        return false;
      }
      Countries[x[0]] = x[1];
    }

    for(count i = 0; i < ProgrammingLanguageList.n(); i++)
    {
      List<String> x = ProgrammingLanguageList[i].Tokenize("\t");
      if(x.n() != 2)
      {
        C::Out() << "Error while parsing: " << ProgrammingLanguageList[i];
        return false;
      }
      ProgrammingLanguages[x[0]] = x[1];
    }

    return true;
  }

  ///Returns the full description of a programming language code.
  String GetProgrammingLanguageCodeDescription(String LangCode) const
  {
    String Result;
    List<String> t = LangCode.Tokenize("-");

    if(t.n() == 1)
    {
      //Only language is specified.
      if(Languages[t[0]] == "")
      {
        C::Out() << "Error: The language code '" << t[0] <<
          "' is unrecognized.";
        C::Out()++;
        C::Out() << "Supported languages are: " >> Languages;
        return "";
      }
      Result = Languages[t[0]];
    }
    else if(t.n() == 2)
    {
      //Language/country or language/programming language is specified.
      if(Languages[t[0]] == "")
      {
        C::Out() << "Error: The language code '" << t[0] <<
          "' is unrecognized.";
        C::Out()++;
        C::Out() << "Supported languages are: " >> Languages;
        return "";
      }
      else if(ProgrammingLanguages[t[1]] == "" && Countries[t[1]] == "")
      {
        C::Out() << "Error: The programming language or country code '" <<
          t[1] << "' is unrecognized.";
        C::Out()++;
        C::Out() << "Supported programming lanuages are: " >>
          ProgrammingLanguages;
        C::Out()++;
        C::Out() << "Supported countries are: " >> Countries;
        return "";
      }

      if(ProgrammingLanguages[t[1]])
        Result = ProgrammingLanguages[t[1]] + " with " +
          Languages[t[0]] + " identifiers";
      else
        Result = Languages[t[0]] + " (" + Countries[t[1]] + ")";
    }
    else if(t.n() == 3)
    {
      //Language/country/programming language is specified.
      if(!Languages[t[0]])
      {
        C::Out() << "Error: The language code '" << t[0] <<
          "' is unrecognized.";
        C::Out()++;
        C::Out() << "Supported languages are: " >> Languages;
        return "";
      }
      else if(!Countries[t[1]])
      {
        C::Out() << "Error: The country code '" << t[1] <<
          "' is unrecognized.";
        C::Out()++;
        C::Out() << "Supported countries are: " >> Countries;
        return "";
      }
      else if(!ProgrammingLanguages[t[2]])
      {
        C::Out() << "Error: The programming language '" << t[2] <<
          "' is unrecognized.";
        C::Out()++;
        C::Out() << "Supported programming languages are: " >>
          ProgrammingLanguages;
        return "";
      }

      Result = ProgrammingLanguages[t[2]] + " with " +
        Languages[t[0]] + " (" + Countries[t[1]] +
        ") identifiers";
    }
    else
    {
      C::Out() << "Error: The language code '" << LangCode <<
        "' could not be parsed.";
      return "";
    }
    return Result;
  }

  /**Returns whether the language code is valid. Examples of valid language
  codes are en and en-GB.*/
  bool IsLanguageCodeValid(String LangCode) const
  {
    if(!LangCode)
      return false;
    return bool(GetProgrammingLanguageCodeDescription(LangCode)) &&
      !GetProgrammingLanguage(LangCode);
  }

  /**Returns whether the programming language code is valid. Examples of valid
  programming language codes are en, en-GB, en-cpp, and en-GB-cpp.*/
  bool IsProgrammingLanguageCodeValid(String LangCode) const
  {
    if(!LangCode)
      return false;
    return GetProgrammingLanguageCodeDescription(LangCode);
  }

  ///Gets the country code if specified.
  String GetCountry(String LangCode) const
  {
    if(!IsProgrammingLanguageCodeValid(LangCode))
      return "";
    List<String> t = LangCode.Tokenize("-");
    if(t.n() >= 2)
      return Countries[t[1]];
    return "";
  }

  ///Gets the language code if specified.
  String GetLanguage(String LangCode) const
  {
    if(!IsProgrammingLanguageCodeValid(LangCode))
      return "";
    List<String> t = LangCode.Tokenize("-");
    return Languages[t[0]];
  }

  ///Gets the language code if specified.
  String GetProgrammingLanguage(String LangCode) const
  {
    if(!IsProgrammingLanguageCodeValid(LangCode))
      return "";
    List<String> t = LangCode.Tokenize("-");
    if(t.n() == 3)
      return ProgrammingLanguages[t[2]];
    else if(t.n() == 2)
      return ProgrammingLanguages[t[1]];
    return "";
  }

  ///Returns a string summary of the language code.
  String GetSummary(String LangCode) const
  {
    if(!IsProgrammingLanguageCodeValid(LangCode))
      return LangCode + " is not a valid language code.";
    String s;
    s >> "Language Code       : " << LangCode;
    s >> "Description         : " <<
      GetProgrammingLanguageCodeDescription(LangCode);
    s >> "Language            : " << GetLanguage(LangCode);
    s >> "Country             : " << GetCountry(LangCode);
    s >> "Programming Language: " << GetProgrammingLanguage(LangCode);
    s >> "Is Natural Language : " << IsLanguageCodeValid(LangCode);
    return s;
  }
};

///Creates an array of whitespace strings.
class Whitespace : public Array<String>
{
  public:

  ///Creates the whitespace array.
  Whitespace()
  {
    //Whitespace according Unicode standard
    Add(String(unicode(0x09)));
    Add(String(unicode(0x0A)));
    Add(String(unicode(0x0B)));
    Add(String(unicode(0x0C)));
    Add(String(unicode(0x0D)));
    Add(String(unicode(0x20)));
    Add(String(unicode(0x85)));
    Add(String(unicode(0xA0)));
    Add(String(unicode(0x1680)));
    Add(String(unicode(0x180E)));
    Add(String(unicode(0x2000)));
    Add(String(unicode(0x2001)));
    Add(String(unicode(0x2002)));
    Add(String(unicode(0x2003)));
    Add(String(unicode(0x2004)));
    Add(String(unicode(0x2005)));
    Add(String(unicode(0x2006)));
    Add(String(unicode(0x2007)));
    Add(String(unicode(0x2008)));
    Add(String(unicode(0x2009)));
    Add(String(unicode(0x200A)));
    Add(String(unicode(0x2028)));
    Add(String(unicode(0x2029)));
    Add(String(unicode(0x202F)));
    Add(String(unicode(0x205F)));
    Add(String(unicode(0x3000)));
  }

  ///Converts all whitespace into the given whitespace character.
  String Normalize(String s, String Replacement = " ")
  {
    for(count i = 0; i < n(); i++)
      s.Replace(ith(i), Replacement);
    return s;
  }

  ///Converts all whitespace to space and collapses multiple spaces into one.
  String Collapse(String s)
  {
    s = Normalize(s, " ");
    while(s.Replace("  ", " "));
    s.Trim();
    return s;
  }
};

///Stores a language-validated localized string.
class LocalizedString
{
  String LanguageCode;
  String LocalString;
  const LocalizationData* Data;

  public:

  ///Creates an empty localized string.
  LocalizedString() : Data(0) {}

  ///Creates a localized string from a language-prefixed string.
  LocalizedString(String s, const LocalizationData& d) : Data(&d)
  {
    count i = s.Find(":");
    if(i <= 0)
      LocalString = LanguageCode = "";
    else
    {
      LanguageCode = s.Substring(0, i - 1);
      LocalString = s.Substring(i + 1, s.n() - 1);
    }

    if(!Data->IsProgrammingLanguageCodeValid(LanguageCode))
      LocalString = LanguageCode = "";
  }

  ///Returns the language-prefixed string.
  operator String () const
  {
    if(!LanguageCode || !LocalString)
      return "";
    return LanguageCode + ":" + LocalString;
  }

  ///Returns whether the language code indicated is a natural language.
  bool IsNaturalLanguage() const
  {
    if(Data)
      return Data->IsLanguageCodeValid(LanguageCode);
    return false;
  }

  ///Returns the language code.
  String Language() const
  {
    return LanguageCode;
  }

  ///Returns the localized text.
  String Text() const
  {
    return LocalString;
  }
};

class UUIDv4Hash
{
  public:

  /*Standard MICA hashing algorithm. Takes in two 64-bit integers and produces
  a single 64-bit integer out. This is used for simplifying map lookup.*/
  static PRIM_NAMESPACE::uint64 Hash(PRIM_NAMESPACE::uint64 a,
    PRIM_NAMESPACE::uint64 b)
  {
    //Use multiply with carry random number generation with inputs as seed.
    const PRIM_NAMESPACE::uint64 low16 = 0xffffUL;
    const PRIM_NAMESPACE::uint64 low32 = 0xffffffffUL;
    PRIM_NAMESPACE::uint64 z_h, w_h, z_l, w_l;
    z_h = 36969 * ((a >> 32) & low16) + (a >> 48);
    w_h = 18000 * ((b >> 32) & low16) + (b >> 48);
    z_l = 36969 * (a & low16) + ((a & low32) >> 16);
    w_l = 18000 * (b & low16) + ((b & low32) >> 16);
    return (((z_l << 16) + w_l) & low32) + (((z_h << 16) + w_h) << 32);
  }

  static UUIDv4 Xor(UUIDv4 a, UUIDv4 b)
  {
    return UUIDv4(Hash(a.High(), b.High()), Hash(a.Low(), b.Low()));
  }

  static String TranslateHexNumber(String s)
  {
    return s.StartsWith("0x") ? String(s.ToHexNumber()) : s;
  }
  static bool IsNumber(String s)
  {
    return s.StartsWith("0x") || !Ratio(s).IsEmpty();
  }

  static bool IsIdentifier(String s)
  {
    return s.Contains(":");
  }

  static bool IsUUIDv4(String s)
  {
    return UUIDv4::IsUUIDv4(s.Merge());
  }

  static bool IsUUIDv4Number(UUIDv4 u)
  {
    return int64(u.Low()) >= 0;
  }

  static bool IsValid(String s)
  {
    UUIDv4 u = Generate(s);
    if(u == UUIDv4(0, 0))
    {
      /*If a null UUIDv4 was returned and it was not explicitly stated, then the
      input string is invalid.*/
      if(s != String(UUIDv4(0, 0)))
        return false;
    }
    return true;
  }

  static Ratio UUIDv4ToNumber(UUIDv4 u)
  {
    if(!IsUUIDv4Number(u))
      return Ratio(0, 0);
    return Ratio(int64(u.High()), int64(u.Low()));
  }

  static UUIDv4 Generate(String s)
  {
    /*The MICA namespace consists of the digits 31ca (representing the letters
    of MICA) repeated 8 times. It is also equivalent to having the high and low
    64-bit portions equal to: 3587734797642510794.*/
    UUIDv4 MICANamespace("31ca31ca-31ca-31ca-31ca-31ca31ca31ca");

    if(IsUUIDv4(s))
      return UUIDv4(s);
    else if(IsNumber(s))
    {
      /*A MICA number is a ratio expressed with the numerator as the high part
      and the denominator as the low part of a UUIDv4.

      nnnnnnnn-nnnn-nnnn-dddd-dddddddddddd = n / d

      The expressible range of a MICA number is -(2^63) <= n < (2^63) and
      0 <= d < (2^63). Note that since the negative sign is applied to the
      numerator and the denominator is always non-negative, this means that in
      two's complement, the denominators high bit will always be zero. This
      corresponds to a variant other than the one used specified by RFC 4122
      (technically the NCS backwards compatibility variant). This means that the
      MICA number domain will not overlap the ID domain.

      Note that the MICA Undefined ID of all zeroes coresponds to the MICA
      number 0 / 0 which is the representation for an empty number.*/
      Ratio r(TranslateHexNumber(s));
      return UUIDv4(uint64(r.Numerator()), uint64(r.Denominator()));
    }
    else if(IsIdentifier(s))
      return UUIDv4(MICANamespace, s);
    return UUIDv4(0, 0);
  }

  static void CheckID(String s)
  {
    C::Out() >> s << ": " << UUIDv4Hash::IsValid(s);
  }

  static void CheckIDs()
  {
    CheckID("en:A");
    CheckID("1");
    CheckID("1/1");
    CheckID("-1/1");
    CheckID("-3/10");
    CheckID("-03/10");
    CheckID("-03/010");
    CheckID("9223372036854775807");
    CheckID("-9223372036854775807/2");
    CheckID("00000000-0000-0000-0000-000000000000");
    CheckID("00112233-4455-6677-8899-aAbBcCdDeEfF");
    CheckID("-9223372036854775807/9223372036854775806");
    C::Out() >> "--";
    CheckID("");
    CheckID("enA");
    CheckID("0/0");
    CheckID("-0/0");
    CheckID("3/-10");
    CheckID("-9223372036854775808");
    CheckID("-9223372036854775808/2");
    CheckID("-9223372036854775808/9223372036854775807");
    CheckID("00112233-4455-6677-8899-aAbBcCdDeEfFX");
    CheckID("00000000-0000-xyzw-0000-000000000000");
  }
};

/**Parses all the MICA XML files. The data gathered here can then be run through
the interpreter.*/
class Parser
{
  public:

  struct Definition
  {
    LocalizedString Name;
    String Description;

    bool IsValid() const
    {
      return bool(String(Name));
    }

    operator String () const
    {
      String s;
      s << Name << " = " << Description;
      return s;
    }
  };

  struct Concept
  {
    List<String> Elements;

    bool Cyclic; PRIM_PAD(bool)

    String UID;

    Array<LocalizedString> Identifiers;

    Array<Definition> Definitions;

    Concept() : Cyclic(false) {}

    operator String() const
    {
      String s;
      s << UID << "-" << Identifiers;
      if(Definitions.n())
        s << "-" << Definitions;
      if(Elements.n())
        s << "-" << Elements << (Cyclic ? "-cyclic" : "");
      s << "";
      return s;
    }
  };

  struct Category
  {
    Array<Definition> Definitions;
    Array<Concept> Concepts;

    operator String () const
    {
      String s;
      s++;
      s >> Definitions;
      s++;
      s >> Concepts;
      return s;
    }
  };

  ///Array containing each category parsed import.
  Array<Category> RawCategoryData;

  ///Array containing each map parsed during import.
  Array<Array<String> > RawMapData;

  /**Imports all the XML files found in the definitions directory. It looks
  through subdirectories as well.*/
  bool Import()
  {
    //Read in the localization data.
    if(!Locale.Import())
    {
      C::Out() >> "Halting.";
      return false;
    }

    //Determine the input set of files recursively over the library.
    Array<String> Files = Parser::RecursiveFiles("definitions");

    //Try looking in the parent directory if no files were found.
    if(!Files.n())
      Files = Parser::RecursiveFiles("../definitions");

    //If still no files were found then halt.
    if(!Files.n())
    {
      C::Out() >> "Error: no files were found in the definitions directory.";
      C::Out() >>
        "       (make sure your executable path contains definitions)";
      C::Out() >> "Halting.";
      return false;
    }

    //Import each XML file.
    C::Out() >> "Parsing...";
    for(count i = 0; i < Files.n(); i++)
    {
      if(!ImportXMLFile(Files[i]))
      {
        C::Out() >> "Halting.";
        return false;
      }
    }
    return true;
  }

  LocalizationData Locale;

  private:

  Whitespace Space;

  static Array<String> RecursiveFiles(String Path)
  {
    Array<String> Files, Directories;
    Directory::Files(Path, Files, Directory::JustFiles, false, "xml");
    Directory::Files(Path, Directories, Directory::JustDirectories);
    for(count i = 0; i < Directories.n(); i++)
    {
      Array<String> Children = RecursiveFiles(Directories[i]);
      for(count j = 0; j < Children.n(); j++)
        Files.Add() = Children[j];
    }
    return Files;
  }

  Array<LocalizedString> ParseLocalizedConceptsList(String s)
  {
    List<String> l = Space.Collapse(s).Tokenize(" ");
    Array<LocalizedString> a;
    for(count i = 0; i < l.n(); i++)
    {
      a.Add() = LocalizedString(l[i], Locale);
      if(!(String(a.z())))
      {
        a.Clear();
        return a;
      }
    }
    return a;
  }

  bool ParseMaps(XML::Element* Root)
  {
    count m = 0;
    for(count j = 0, n = Root->GetObjects().n(); j < n; j++)
    {
      XML::Element* e = dynamic_cast<XML::Element*>(Root->GetObjects()[j]);
      if(!e) continue;
      if(e->GetName() != "map") continue;
      if(m++ % 10000 == 5000)
        C::Out() << ".";
      List<String> t =
        Space.Collapse(e->GetAllSubTextAsString()).Tokenize(" ");
      Array<String> mArray;
      for(count i = 0; i < t.n(); i++)
      {
        mArray.Add(t[i]);
        if(UUIDv4Hash::IsNumber(t[i]))
        {
        }
        else if(UUIDv4Hash::IsIdentifier(t[i]))
        {
          if(!String(LocalizedString(t[i], Locale)))
          {
            C::Out()++;
            C::Out() >> "Error: invalid map identifier " << t[i];
            C::Out()++;
            return false;
          }
        }
        else
        {
          C::Out()++;
          C::Out() >> "Error: invalid map token " << t[i];
          C::Out()++;
          return false;
        }
      }
      if(mArray.n() < 3)
      {
        C::Out()++;
        C::Out() >> "Error: map contains less than three elements: '" <<
          e->GetAllSubTextAsString() << "'";
        C::Out()++;
        return false;
      }
      RawMapData.Add() = mArray;
    }
    return true;
  }

  Definition ParseDefinition(XML::Element* DefinitionElement)
  {
    Definition d;
    String l = DefinitionElement->GetAttributeValue("lang");
    String n = DefinitionElement->GetAttributeValue("name");
    d.Name = LocalizedString(l + ":" + n, Locale);
    String s = DefinitionElement->GetAllSubTextAsString();
    if(DefinitionElement->GetAttributeValue("xml:space") != "preserve")
      s = Space.Collapse(s);
    d.Description = s;
    return d;
  }

  LocalizedString ParseIdentifier(XML::Element* IdentifierElement)
  {
    LocalizedString s;
    String l = IdentifierElement->GetAttributeValue("lang");
    String n = Space.Collapse(IdentifierElement->GetAllSubTextAsString());
    if(n.Contains(" "))
    {
      C::Out() >> "Identifier '" << n << "' contains illegal whitespace.";
      return s;
    }
    s = LocalizedString(l + ":" + n, Locale);
    return s;
  }

  bool ParseConcept(Category& cat, XML::Element* ConceptElement)
  {
    //concepts may have sequence, definition, identifier

    //Add a concept to the current category.
    Concept& con = cat.Concepts.Add();

    //Retrieve the unique ID.
    con.UID = ConceptElement->GetAttributeValue("uuid");

    //Check to make sure the UID exists.
    if(!con.UID)
    {
      C::Out() >> "Error: concept without uuid attribute";
      return false;
    }

    //Check to see if the identifier is a UUIDv4 or a localized identifier.
    bool IsUUIDv4 = (UUIDv4(con.UID) != UUIDv4(0, 0)) ||
      (con.UID == String(UUIDv4(0, 0)));
    bool IsIdentifier = String(LocalizedString(con.UID, Locale));

    //If not an identifier or UUIDv4 then halt.
    if(!IsIdentifier && !IsUUIDv4)
    {
      C::Out() >> "Error: " << con.UID << " is not a valid UUIDv4 or localized "
        "identifier.";
      return false;
    }

    //Check to make sure the UUIDv4 is in lowercase.
    if(IsUUIDv4 && con.UID != String(UUIDv4(con.UID)))
    {
      C::Out() >> "Error: " << con.UID <<
        " is not in canonical lowercase form.";
      return false;
    }

    //Add sequence information if it exists.
    for(count i = 0, n = ConceptElement->CountChildrenByTagName(
      "sequence"); i < n; i++)
    {
      if(n > 1)
      {
        C::Out() >>
            "Error: a concept may not have more than one sequence definition.";
        return false;
      }

      //Retrieve the sequence element.
      XML::Element* e = ConceptElement->GetChildByTagName("sequence", i);

      //Get the concept list out of the sequence.
      con.Elements = Space.Collapse(e->GetAllSubTextAsString()).Tokenize(" ");

      //Parse the cyclic attribute.
      String cyclic = e->GetAttributeValue("cyclic");
      if(cyclic == "true")
        con.Cyclic = true;
      else if(cyclic == "false")
      {
        C::Out() >> "Warning: sequence attribute cyclic defaults to 'false'.";
        con.Cyclic = false;
      }
      else if(cyclic == "")
        con.Cyclic = false;
      else
      {
        C::Out() >>
            "Error: sequence attribute cyclic with value other than 'true': "
            << cyclic;
        return false;
      }
    }

    //Parse all definitions.
    for(count i = 0, n = ConceptElement->CountChildrenByTagName(
      "definition"); i < n; i++)
    {
      Definition d =
        ParseDefinition(ConceptElement->GetChildByTagName("definition", i));
      if(!d.IsValid())
      {
        C::Out() >> "Error: malformed definition.";
        return false;
      }
      con.Definitions.Add(d);
    }

    //Parse all identifiers.
    for(count i = 0, n = ConceptElement->CountChildrenByTagName(
      "identifier"); i < n; i++)
    {
      LocalizedString iden =
        ParseIdentifier(ConceptElement->GetChildByTagName("identifier", i));
      if(!String(iden))
      {
        C::Out() >> "Error: malformed identifier.";
        return false;
      }
      con.Identifiers.Add(iden);
    }

    if(!con.Identifiers.n())
    {
      C::Out() >> "Error: concept contains no identifiers.";
      return false;
    }

    return true;
  }

  bool ParseCategory(XML::Element* CategoryElement)
  {
    Category& cat = RawCategoryData.Add();

    //Parse all definitions.
    for(count i = 0, n = CategoryElement->CountChildrenByTagName(
      "definition"); i < n; i++)
    {
      Definition d =
        ParseDefinition(CategoryElement->GetChildByTagName("definition", i));
      if(!d.IsValid())
      {
        C::Out() >> "Error: malformed definition.";
        return false;
      }
      cat.Definitions.Add(d);
    }

    //Parse all concepts.
    for(count i = 0, n = CategoryElement->CountChildrenByTagName(
      "concept"); i < n; i++)
        if(!ParseConcept(cat, CategoryElement->GetChildByTagName("concept", i)))
          return false;
    return true;
  }

  bool ParseCategories(XML::Element* Root)
  {
    for(count i = 0, n = Root->CountChildrenByTagName("category"); i < n; i++)
      if(!ParseCategory(Root->GetChildByTagName("category", i)))
        return false;
    return true;
  }

  bool ImportXMLFile(String Filename)
  {
    //Read in the file.
    C::Out() >> " * " << Filename << " ...";
    String s;
    File::Read(Filename, s);

    //Check for an empty file and skip.
    if(!s)
    {
      C::Out() << " (warning: empty file)";
      return true;
    }

    //Parse the XML and halt if there was an error.
    XML::Document d;
    if(XML::Parser::Error e = d.ParseDocument(s))
    {
      C::Out() >> e.GetDescription();
      return false;
    }

    //Check to make sure the root element is sane.
    if(!d.Root || d.Root->GetName() != "mica")
    {
      C::Out() >> "Error: Root element is not <mica> or does not exist.";
      return false;
    }

    //Parse the maps and halt on error.
    if(!ParseMaps(d.Root))
      return false;

    //Parse the categories and halt on error.
    if(!ParseCategories(d.Root))
      return false;

    return true;
  }
};

/**Parses all the MICA XML files. The data gathered here can then be run through
the interpreter.*/
class Interpreter
{
  public:

  struct Concept;

  struct Category
  {
    Tree<String> Definitions;
    Array<Pointer<Concept> > Concepts;
  };

  struct Concept
  {
    UUIDv4 ID;

    Tree<String> Identifiers;
    Tree<String> Definitions;
    bool Cyclic; PRIM_PAD(bool)
    Array<UUIDv4> Elements;
    Pointer<Category> Parent;
    Parser::Concept OriginalConcept;
    List< Array<UUIDv4> > Mappings;

    Concept() : Cyclic(false) {}
  };

  struct SequenceConcept
  {
    UUIDv4 SequenceItem;
    UUIDv4 ConceptItem;

    SequenceConcept() : SequenceItem(UUIDv4::Empty()),
      ConceptItem(UUIDv4::Empty()) {}

    SequenceConcept(UUIDv4 s, UUIDv4 c) : SequenceItem(s), ConceptItem(c) {}

    bool operator < (const SequenceConcept& Other) const
    {
      return SequenceItem < Other.SequenceItem ||
        (SequenceItem == Other.SequenceItem && ConceptItem < Other.ConceptItem);
    }

    bool operator > (const SequenceConcept& Other) const
    {
      return SequenceItem > Other.SequenceItem ||
        (SequenceItem == Other.SequenceItem && ConceptItem > Other.ConceptItem);
    }
  };

  struct SequenceIndex
  {
    UUIDv4 SequenceItem;
    integer Index;

    SequenceIndex() : SequenceItem(UUIDv4::Empty()), Index(0) {}

    SequenceIndex(UUIDv4 s, integer i) : SequenceItem(s), Index(i) {}

    bool operator < (const SequenceIndex& Other) const
    {
      return SequenceItem < Other.SequenceItem ||
        (SequenceItem == Other.SequenceItem && Index < Other.Index);
    }

    bool operator > (const SequenceIndex& Other) const
    {
      return SequenceItem > Other.SequenceItem ||
        (SequenceItem == Other.SequenceItem && Index > Other.Index);
    }
  };

  struct ConceptLanguage
  {
    UUIDv4 ConceptItem;
    String Language;

    ConceptLanguage() : ConceptItem(UUIDv4::Empty()) {}

    ConceptLanguage(UUIDv4 c, String l) : ConceptItem(c), Language(l) {}

    bool operator < (const ConceptLanguage& Other) const
    {
      return ConceptItem < Other.ConceptItem ||
        (ConceptItem == Other.ConceptItem && Language < Other.Language);
    }

    bool operator > (const ConceptLanguage& Other) const
    {
      return ConceptItem > Other.ConceptItem ||
        (ConceptItem == Other.ConceptItem && Language > Other.Language);
    }

    operator String()
    {
      return String(ConceptItem) + "-" + Language;
    }
  };

  const LocalizationData& Locale;

  Tree<String, Pointer<Category> > DefinitionNameToCategory;
  Tree<String, Pointer<Concept> > IdentifierToConcept;
  Tree<String, Pointer<Concept> > DefinitionNameToConcept;

  Array<Pointer<Category> > Categories;
  Array<Pointer<Concept> > Concepts;
  Array<Pointer<Concept> > Sequences;

  Tree<String> NaturalLanguages;
  Tree<String> ProgrammingLanguages;

  Tree<UUIDv4> HashedMappings;
  Tree<UUIDv4, Pointer<Concept> > IDToConcept;
  Tree<SequenceConcept, integer> SequenceConceptToIndex;
  Tree<SequenceIndex, UUIDv4> SequenceIndexToConcept;
  Tree<ConceptLanguage, String> ConceptLanguageToKey;

  Interpreter(Parser& p) : Locale(p.Locale) {}

  bool CreateCategories(Parser& p)
  {
    for(count i = 0; i < p.RawCategoryData.n(); i++)
    {
      Parser::Category& cat = p.RawCategoryData[i];
      if(!cat.Definitions.n())
      {
        C::Out() >> "Error: category with no definition elements.";
        return false;
      }
      Pointer<Category> NewCategory = new Interpreter::Category;
      Categories.Add() = NewCategory;

      for(count j = 0; j < cat.Definitions.n(); j++)
      {
        String n = cat.Definitions[j].Name;
        String d = cat.Definitions[j].Description;
        String l = cat.Definitions[j].Name.Language();
        NaturalLanguages[l] = l;
        if(NewCategory->Definitions[n])
        {
          C::Out() >> "Error: duplicate definition " << n <<
            " within single category";
          return false;
        }
        NewCategory->Definitions[n] = d;

        if(DefinitionNameToCategory[n])
        {
          C::Out() >> "Error: duplicate definition " << n <<
            " across multiple categories";
          return false;
        }
        DefinitionNameToCategory[n] = NewCategory;
      }
    }
    return true;
  }

  bool CreateConcepts(Parser& p)
  {
    for(count i = 0; i < p.RawCategoryData.n(); i++)
    {
      Parser::Category& cat = p.RawCategoryData[i];
      String CategoryName = cat.Definitions[0].Name;
      Pointer<Category> CategoryPointer =
        DefinitionNameToCategory[CategoryName];
      if(!cat.Concepts.n())
        C::Out() >> "Warning: category " <<
        CategoryName << " with no concepts.";
      for(count j = 0; j < cat.Concepts.n(); j++)
      {
        Parser::Concept& con = cat.Concepts[j];
        Pointer<Concept> NewConcept = new Interpreter::Concept;
        NewConcept->Parent = CategoryPointer;
        NewConcept->OriginalConcept = con;
        CategoryPointer->Concepts.Add() = NewConcept;
        Concepts.Add() = NewConcept;

        if(Concepts.n() % 100 == 0) C::Out() << "."; //Progress indicator

        for(count k = 0; k < con.Identifiers.n(); k++)
        {
          String iden = con.Identifiers[k];
          String lang = con.Identifiers[k].Language();
          if(IdentifierToConcept[iden])
          {
            C::Out() >> "Error: duplicate identifier " << iden;
            C::Out() >> "  (first occurrence in: " <<
              IdentifierToConcept[iden]->Identifiers << ")";
            return false;
          }
          IdentifierToConcept[iden] = NewConcept;
          ProgrammingLanguages[lang] = lang;
          NewConcept->Identifiers[iden] = iden;
        }

        for(count k = 0; k < con.Definitions.n(); k++)
        {
          String n = con.Definitions[k].Name;
          String d = con.Definitions[k].Description;
          String l = con.Definitions[k].Name.Language();
          if(DefinitionNameToConcept[n])
          {
            C::Out() >> "Error: duplicate definition " << n;
            C::Out() >> "  (first occurrence in: " <<
              DefinitionNameToConcept[n]->Identifiers << ")";
            return false;
          }
          DefinitionNameToConcept[n] = NewConcept;
          NaturalLanguages[l] = l;
          NewConcept->Definitions[n] = d;
        }

        if(!con.Identifiers.n())
        {
          C::Out() >> "Error: concept with no identifiers.";
          return false;
        }

        String u = con.UID;
        NewConcept->ID = UUIDv4Hash::Generate(u);
        for(count k = 0; k < con.Definitions.n(); k++)
        {
          String n = con.Definitions[k].Name;
          String l = con.Definitions[k].Name.Language();
          ConceptLanguageToKey[ConceptLanguage(NewConcept->ID, l)] = n;
        }
        IDToConcept[NewConcept->ID] = NewConcept;
        bool IsUUIDv4Already = (String(NewConcept->ID) == u);
        if(!IsUUIDv4Already && !NewConcept->Identifiers[u])
        {
          C::Out() >> "Error: concept uses unknown automatic identifier " << u;
          C::Out() >>
            "If the original identifier has been deprecated, replace it";
          C::Out() >>
            "with the UUIDv4 it assigned to and move the identifier to the";
          C::Out() >> "deprecated category.";
          return false;
        }
      }
    }
    return true;
  }

  bool CreateSequences()
  {
    for(count i = 0; i < Concepts.n(); i++)
    {
      //Look for concepts which have elements (meaning they are a sequence).
      Parser::Concept& con = Concepts[i]->OriginalConcept;
      if(!con.Elements.n())
        continue;

      //Check whether the concept is cyclic.
      Concepts[i]->Cyclic = con.Cyclic;

      //Add a new sequence.
      Sequences.Add() = Concepts[i];

      /*Go through each referenced element and add it to the sequence. Note that
      Undefined is currently a valid element.*/
      for(count j = 0; j < con.Elements.n(); j++)
      {
        String s = con.Elements[j];
        if(UUIDv4Hash::IsIdentifier(s))
        {
          if(Pointer<Concept> c = IdentifierToConcept[s])
            Concepts[i]->Elements.Add() = c->ID;
          else
          {
            C::Out() >> "Error: concept identifier " << s << " does not exist.";
            return false;
          }
        }
        else if(UUIDv4Hash::IsNumber(s))
          Concepts[i]->Elements.Add() = UUIDv4Hash::Generate(s);
        else
        {
          C::Out() >> "Error: sequence with invalid sequence element: " << s;
          return false;
        }
      }

      /*Create the sequence-concept to index and sequence-index to concept
      tables.*/
      for(count j = 0; j < Concepts[i]->Elements.n(); j++)
      {
        SequenceConceptToIndex[SequenceConcept(Concepts[i]->ID,
          Concepts[i]->Elements[j])] = j;
        SequenceIndexToConcept[SequenceIndex(Concepts[i]->ID, j)] =
          Concepts[i]->Elements[j];
      }
    }
    return true;
  }

  bool CreateMappings(Parser& p)
  {
    count TotalMappings = 0;
    for(count i = 0; i < p.RawMapData.n(); i++)
    {
      Sortable::Array<UUIDv4> IDs;
      for(count j = 0; j < p.RawMapData[i].n(); j++)
      {
        String s = p.RawMapData[i][j];
        if(UUIDv4Hash::IsNumber(s))
          IDs.Add() = UUIDv4Hash::Generate(s);
        else
        {
          if(Pointer<Concept> cn = IdentifierToConcept[s])
            IDs.Add() = cn->ID;
          else
          {
            C::Out() >> "Error: The map element " << s <<
                " has not been defined.";
            return false;
          }
        }
      }

      Sortable::Array<UUIDv4> ConceptMappings, SortedMappings;
      for(count j = 0; j < IDs.n(); j++)
        ConceptMappings.Add() = IDs[j];
      SortedMappings = ConceptMappings;
      SortedMappings.Sort();
      for(count j = 0; j < SortedMappings.n(); j++)
      {
        UUIDv4 u = SortedMappings[j];
        if(j != 0 && u == SortedMappings[j - 1])
          continue; //Do not add the same ID twice.
        if(!UUIDv4Hash::IsUUIDv4Number(u))
          IDToConcept[u]->Mappings.Add() = ConceptMappings;
      }

      Sortable::Array<UUIDv4> SortedUUIDv4s = IDs;
      SortedUUIDv4s.n(SortedUUIDv4s.n() - 1);
      SortedUUIDv4s.Sort();
      UUIDv4 a = SortedUUIDv4s.a();
      for(count j = 1; j < SortedUUIDv4s.n(); j++)
        a = UUIDv4Hash::Xor(a, SortedUUIDv4s[j]);

      if(HashedMappings[a] != UUIDv4::Empty())
      {
        UUIDv4 u = HashedMappings[a];
        String OldDef, NewDef;
        OldDef << u << " ";
        if(!UUIDv4Hash::IsUUIDv4Number(u))
          OldDef << IDToConcept[u]->Identifiers;
        else if(UUIDv4Hash::IsNumber(u))
          OldDef << UUIDv4Hash::UUIDv4ToNumber(u);

        UUIDv4 u2 = IDs.z();
        NewDef << u2 << " ";
        if(!UUIDv4Hash::IsUUIDv4Number(u2))
        {
          NewDef << IDToConcept[u2]->Identifiers;
        }
        else if(UUIDv4Hash::IsNumber(u2))
          NewDef << UUIDv4Hash::UUIDv4ToNumber(u2);

        C::Out() >> "Error: The unordered map " << p.RawMapData[i] <<
          " has already been defined.";
        C::Out()++;
        C::Out() >>
            "Note that maps are unordered and different orderings of the";
        C::Out() >> "initial elements (all but last) are treated as the same.";
        C::Out()++;
        C::Out() >> "Lookup Hash:         " << a;
        C::Out() >> "Original Definition: " << OldDef;
        C::Out() >> "New Definition:      " << NewDef;
        C::Out()++;
        if(HashedMappings[a] == IDs.z())
          C::Out() >> "Second definition is a redefinition.";
        else
        {
          C::Out() >>
            "Second definition differs! Map lookups are created using a";
          C::Out() >>
            "randomized hash on the inputs. It is possible that a hash";
          C::Out() >> "collision has occurred.";
        }
        C::Out()++;
        return false;
      }
      HashedMappings[a] = IDs.z();
      if(TotalMappings++ % 100 == 0) C::Out() << "."; //Progress indicator
    }
    return true;
  }

  bool Interpret(Parser& p)
  {
    C::Out() >> "Interpreting...";

    C::Out() >> " * Categories: ";
    if(!CreateCategories(p))
    {
      C::Out() >> "Halting.";
      return false;
    }
    C::Out() << Categories.n();

    C::Out() >> " * Concepts:   ";
    if(!CreateConcepts(p))
    {
      C::Out() >> "Halting.";
      return false;
    }
    C::Out() << Concepts.n();

    C::Out() >> " * Sequences:  ";
    if(!CreateSequences())
    {
      C::Out() >> "Halting.";
      return false;
    }
    C::Out() << Sequences.n();

    C::Out() >> " * Mappings:   ";
    if(!CreateMappings(p))
    {
      C::Out() >> "Halting.";
      return false;
    }
    C::Out() << HashedMappings.n();

    return true;
  }
};

class PortBase
{
  public:

  virtual void Create(Interpreter& i, Tree<String>& TemplateIn,
    Tree<String>& TemplateOut) = 0;
  virtual ~PortBase();
};

class Port_en_java : public PortBase
{
  String ToString(PRIM_NAMESPACE::UUIDv4 u)
  {
    String s = u;
    s.Replace("-", "");
    s.Insert("U, 0x", 16);
    s.Prepend("0x");
    s.Append("U");
    return s;
  }

  String ToString(Interpreter& in, PRIM_NAMESPACE::String s)
  {
    return LocalizedString(s, in.Locale).Text();
  }

  static void StreamData(count& index, String& data, String& out,
    bool End = false)
  {
    if(data.n() < 10000 && !End)
      return;
    out >> "    static private void initialize" << index++ << "() {";
    out >> data;
    out >> "    }";
    data = "";
    out++;
    if(End)
    {
      out >> "    static private void initializeAll() {";
      for(count i = 0; i < index; i++)
        out >> "        initialize" << i << "();";
      out >> "    }";
    }
  }

  String Primary;
  String Secondary;

  public:

  Port_en_java() : Primary("en:"), Secondary("en:") {}

  virtual ~Port_en_java();

  String GetKeyByLanguage(Tree<String>& t)
  {
    Array<String> tKeys = t.Keys();
    for(count i = 0, n = t.n(); i < n; i++)
    {
      String k = tKeys[i];
      if(Primary && k.StartsWith(Primary))
        return k;
      if(Secondary && k.StartsWith(Secondary))
        return k;
    }
    return "";
  }

  String Localized(String s)
  {
    if(Primary) s.EraseBeginning(Primary);
    if(Secondary) s.EraseBeginning(Secondary);
    return s;
  }

  static String FilterDefinition(String s)
  {
    s.Replace("\n\n", "@@@@@PARAGRAPH BREAK@@@@@");
    s.Replace("\n", " ");
    while(s.Replace("  ", " ")) {}
    s.Trim();
    s.Replace("\"", "\\\"");
    s.Replace("@@@@@PARAGRAPH BREAK@@@@@", "\\n");
    return s;
  }

  void Create(Interpreter& in, Tree<String>& TemplateIn,
    Tree<String>& TemplateOut)
  {
    String data_java = TemplateIn["mica-en-java.java"];
    String data_java_data = TemplateIn["mica-en-java-data.java"];
    String data_java_definitions = TemplateIn["mica-en-java-definitions.java"];
    String data1, data, data3;
    count data_index = 0;
    count data3_index = 0;
    String data_out;
    String data3_out;

    //Definitions of identifiers
    {
      Array<String> IdentifierToConceptKeys = in.IdentifierToConcept.Keys();
      Array<Pointer<Interpreter::Concept> > IdentifierToConceptValues =
        in.IdentifierToConcept.Values();
      for(count i = 0; i < IdentifierToConceptKeys.n(); i++)
      {
        data1 >> "    public static final UUIDv4 " <<
          ToString(in, IdentifierToConceptKeys[i]) << " =" >>
          "        new UUIDv4(0x" <<
          IdentifierToConceptValues[i]->ID.HighHex() << "L, 0x" <<
          IdentifierToConceptValues[i]->ID.LowHex() << "L);";
        StreamData(data_index, data, data_out);
      }
    }

    //Key -> Concept : _key__concept
    {
      Array<String> DefinitionNameToConceptKeys =
        in.DefinitionNameToConcept.Keys();
      Array<Pointer<Interpreter::Concept> > DefinitionNameToConceptValues =
        in.DefinitionNameToConcept.Values();
      for(count i = 0; i < DefinitionNameToConceptKeys.n(); i++)
      {
        data >>
          "        /*KC" << i << "*/ _key__concept.put(\"" <<
          DefinitionNameToConceptKeys[i] << "\"," >>
          "            new UUIDv4(0x" <<
          DefinitionNameToConceptValues[i]->ID.HighHex() << "L, 0x" <<
          DefinitionNameToConceptValues[i]->ID.LowHex() << "L));";
        StreamData(data_index, data, data_out);
      }
      data++;
    }

    //Concept -> Definition : _concept__definition
    Array<UUIDv4> IDToConceptKeys = in.IDToConcept.Keys();
    Array<Pointer<Interpreter::Concept> > IDToConceptValues =
      in.IDToConcept.Values();
    {
      for(count i = 0; i < IDToConceptKeys.n(); i++)
      {
        UUIDv4 u = IDToConceptKeys[i];
        Pointer<Interpreter::Concept> c = IDToConceptValues[i];
        String Definition = FilterDefinition(Localized(c->Definitions[
          GetKeyByLanguage(c->Definitions)]));
        data3 >>
          "        /*CD" << i << "*/ _concept__definitions.put(" >>
          "            new UUIDv4(0x" << u.HighHex() <<
          "L, 0x" << u.LowHex() << "L)," >>
          "            \"" << Definition << "\");";
        StreamData(data3_index, data3, data3_out);
      }
      data3++;
    }

    //Concept -> Length : _concept__length
    {
      for(count i = 0; i < IDToConceptKeys.n(); i++)
      {
        data >>
          "        /*CL" << i << "*/ _concept__length.put(" >>
          "            new UUIDv4(0x" <<
          IDToConceptKeys[i].HighHex() << "L, 0x" <<
          IDToConceptKeys[i].LowHex() << "L)," >>
          "            new Integer(" <<
          (IDToConceptValues[i]->Cyclic ? "-" : "") <<
          IDToConceptValues[i]->Elements.n() << "));";
        StreamData(data_index, data, data_out);
      }
      data++;
    }

    //{Sequence, Concept} -> Index : _sequenceConcept__index
    {
      Array<Interpreter::SequenceConcept> SequenceConceptKeys;
      SequenceConceptKeys = in.SequenceConceptToIndex.Keys();
      Array<integer> SequenceConceptValues;
      SequenceConceptValues = in.SequenceConceptToIndex.Values();
      for(count i = 0; i < SequenceConceptKeys.n(); i++)
      {
        data >>
          "        /*SCI" << i << "*/ _sequenceConcept__index.put("
          "new MICASequenceConcept(" >>
          "            new UUIDv4(0x" <<
          SequenceConceptKeys[i].SequenceItem.HighHex() <<
          "L, 0x" <<
          SequenceConceptKeys[i].SequenceItem.LowHex() <<
          "L)," >> "            new UUIDv4(0x" <<
          SequenceConceptKeys[i].ConceptItem.HighHex() <<
          "L, 0x" <<
          SequenceConceptKeys[i].ConceptItem.LowHex() <<
          "L))," >>
          "            new Integer(" <<
          SequenceConceptValues[i] << "));";
        StreamData(data_index, data, data_out);
      }
      data++;
    }

    //{Sequence, Index} -> Concept : _sequenceIndex__concept
    {
      Array<Interpreter::SequenceIndex> SequenceIndexKeys;
      SequenceIndexKeys = in.SequenceIndexToConcept.Keys();
      Array<UUIDv4> SequenceIndexValues;
      SequenceIndexValues = in.SequenceIndexToConcept.Values();
      for(count i = 0; i < SequenceIndexKeys.n(); i++)
      {
        data >>
          "        /*SIC" << i << "*/ _sequenceIndex__concept.put(" <<
          "new MICASequenceIndex(" >>
          "            new UUIDv4(0x" <<
          SequenceIndexKeys[i].SequenceItem.HighHex() <<
          "L, 0x" <<
          SequenceIndexKeys[i].SequenceItem.LowHex() <<
          "L)," >>
          "            new Integer(" <<
          SequenceIndexKeys[i].Index << "))," >>
          "            new UUIDv4(0x" <<
          SequenceIndexValues[i].HighHex() << "L, 0x" <<
          SequenceIndexValues[i].LowHex() << "L));";
        StreamData(data_index, data, data_out);
      }
      data++;
    }

    //{Concept, Language} -> Key : _conceptLanguage__key
    {
      Array<Interpreter::ConceptLanguage> ConceptLanguageKeys =
        in.ConceptLanguageToKey.Keys();
      Array<String> ConceptLanguageValues =
        in.ConceptLanguageToKey.Values();
      for(count i = 0; i < ConceptLanguageKeys.n(); i++)
      {
        data >>
          "        /*CLK" << i << "*/ _conceptLanguage__key.put(" <<
          "new MICAConceptLanguage(" >>
          "            new UUIDv4(0x" <<
          ConceptLanguageKeys[i].ConceptItem.HighHex() <<
          "L, 0x" <<
          ConceptLanguageKeys[i].ConceptItem.LowHex() <<
          "L), \"" << ConceptLanguageKeys[i].Language << "\")," >>
          "            \"" << ConceptLanguageValues[i] << "\");";
        StreamData(data_index, data, data_out);
      }
      data++;
    }

    //Map Hashed Concepts -> Concept : _hashedConcept__concept
    {
      Array<UUIDv4> HashedMappingsKeys = in.HashedMappings.Keys();
      Array<UUIDv4> HashedMappingsValues = in.HashedMappings.Values();
      for(count i = 0; i < HashedMappingsKeys.n(); i++)
      {
        data >>
          "        /*MC" << i << "*/ _hashedConcept__concept.put(" >>
          "            new UUIDv4(0x" <<
            HashedMappingsKeys[i].HighHex() << "L, 0x" <<
            HashedMappingsKeys[i].LowHex() << "L)," >>
          "            new UUIDv4(0x" <<
            HashedMappingsValues[i].HighHex() << "L, 0x" <<
            HashedMappingsValues[i].LowHex() << "L));";
        StreamData(data_index, data, data_out);
      }
    }
    StreamData(data_index, data, data_out, true);
    StreamData(data3_index, data3, data3_out, true);

    //Insert the data into the templates.
    data_java.Replace("@@@@@1@@@@@", data1);
    data_java_data.Replace("@@@@@2@@@@@", data_out);
    data_java_definitions.Replace("@@@@@3@@@@@", data3_out);

    //Create the export file.
    TemplateOut["MICA/MICA.java"] = data_java;
    TemplateOut["MICA/MICAData.java"] = data_java_data;
    TemplateOut["MICA/MICADefinitions.java"] = data_java_definitions;
  }
};

////////////////////////////////////////////////////////////////////////////////

class Port_en_cpp : public PortBase
{
  String ToString(PRIM_NAMESPACE::UUIDv4 u)
  {
    String s = u;
    s.Replace("-", "");
    s.Insert("U, 0x", 16);
    s.Prepend("0x");
    s.Append("U");
    return s;
  }

  String ToString(Interpreter& in, PRIM_NAMESPACE::String s)
  {
    return LocalizedString(s, in.Locale).Text();
  }

  public:

  virtual ~Port_en_cpp();

  void Create(Interpreter& in, Tree<String>& TemplateIn,
    Tree<String>& TemplateOut)
  {
    String data1, data;

    //Definitions of identifiers
    {
      Array<String> IdentifierToConceptKeys = in.IdentifierToConcept.Keys();
      Array<Pointer<Interpreter::Concept> > IdentifierToConceptValues =
        in.IdentifierToConcept.Values();
      for(count i = 0; i < IdentifierToConceptKeys.n(); i++)
      {
        data1 >> "const UUIDv4 " <<
          ToString(in, IdentifierToConceptKeys[i]) << " = {" <<
          ToString(IdentifierToConceptValues[i]->ID) << "};";
      }
      data1++;
    }

    data >> "namespace mica_internal";
    data >> "{";

    //Key -> Concept
    {
      Array<String> DefinitionNameToConceptKeys =
        in.DefinitionNameToConcept.Keys();
      Array<Pointer<Interpreter::Concept> > DefinitionNameToConceptValues =
        in.DefinitionNameToConcept.Values();
      count n = DefinitionNameToConceptKeys.n();
      data >> "static const int Key_Concept_Size = " << n << ";";
      data >> "static const Key_Concept Key_Concept_Lookup[" <<
        "Key_Concept_Size] = {";
      for(count i = 0; i < DefinitionNameToConceptKeys.n(); i++)
        data >>
          "/*KC" << i << "*/ {\"" <<
          DefinitionNameToConceptKeys[i] << "\", {" <<
          ToString(DefinitionNameToConceptValues[i]->ID) << "}}" <<
          ((i + 1 < n) ? "," : "");
      data >> "};";
      data++;
    }

    //Concept -> Length
    {
      count n = in.IDToConcept.n();
      data >> "static const int Concept_Length_Size = " << n << ";";
      data >> "static const Concept_Length Concept_Length_Lookup[" <<
        "Concept_Length_Size] = {";

      Array<UUIDv4> IDToConceptKeys = in.IDToConcept.Keys();
      Array<Pointer<Interpreter::Concept> > IDToConceptValues =
        in.IDToConcept.Values();
      for(count i = 0; i < IDToConceptKeys.n(); i++)
        data >>
          "/*CL" << i << "*/ {{" <<
          ToString(IDToConceptKeys[i]) << "}, " <<
          (IDToConceptValues[i]->Cyclic ? "-" : "") <<
          IDToConceptValues[i]->Elements.n() << "}" <<
          ((i + 1 < n) ? "," : "");
      data >> "};";
      data++;
    }

    //{Sequence, Concept} -> Index
    {
      count n = in.SequenceConceptToIndex.n();
      data >> "static const int SequenceConcept_Index_Size = " << n << ";";
      data >> "static const SequenceConcept_Index SequenceConcept_Index_Lookup["
        << "SequenceConcept_Index_Size] = {";

      Array<Interpreter::SequenceConcept> SequenceConceptKeys;
      SequenceConceptKeys = in.SequenceConceptToIndex.Keys();
      Array<integer> SequenceConceptValues;
      SequenceConceptValues = in.SequenceConceptToIndex.Values();
      for(count i = 0; i < SequenceConceptKeys.n(); i++)
        data >>
          "/*SCI" << i << "*/ {{" <<
          ToString(SequenceConceptKeys[i].SequenceItem) <<
            "}, {"
          << ToString(SequenceConceptKeys[i].ConceptItem) <<
            "}, "
          << SequenceConceptValues[i] << "}" <<
          ((i + 1 < n) ? "," : "");
      data >> "};";
      data++;
    }

    //{Sequence, Index} -> Concept
    {
      count n = in.SequenceIndexToConcept.n();
      data >> "static const int SequenceIndex_Concept_Size = " << n << ";";
      data >> "static const SequenceIndex_Concept SequenceIndex_Concept_Lookup["
        << "SequenceIndex_Concept_Size] = {";

      Array<Interpreter::SequenceIndex> SequenceIndexKeys;
      SequenceIndexKeys = in.SequenceIndexToConcept.Keys();
      Array<UUIDv4> SequenceIndexValues;
      SequenceIndexValues = in.SequenceIndexToConcept.Values();
      for(count i = 0; i < SequenceIndexKeys.n(); i++)
        data >>
          "/*SIC" << i << "*/ {{" <<
          ToString(SequenceIndexKeys[i].SequenceItem) << "}, "
          << SequenceIndexKeys[i].Index << ", {"
          << ToString(SequenceIndexValues[i]) << "}}" <<
          ((i + 1 < n) ? "," : "");
      data >> "};";
      data++;
    }

    //{Concept, Language} -> Key
    {
      count n = in.ConceptLanguageToKey.n();
      data >> "static const int ConceptLanguage_Key_Size = " << n << ";";
      data >> "static const ConceptLanguage_Key ConceptLanguage_Key_Lookup[" <<
        "ConceptLanguage_Key_Size] = {";

      Array<Interpreter::ConceptLanguage> ConceptLanguageKeys =
        in.ConceptLanguageToKey.Keys();
      Array<String> ConceptLanguageValues =
        in.ConceptLanguageToKey.Values();
      for(count i = 0; i < ConceptLanguageKeys.n(); i++)
        data >>
          "/*CLK" << i << "*/ {{" <<
          ToString(ConceptLanguageKeys[i].ConceptItem) << "}, \""
          << ConceptLanguageKeys[i].Language << "\", \"" <<
          ConceptLanguageValues[i] << "\"}" <<
          ((i + 1 < n) ? "," : "");
      data >> "};";
      data++;
    }

    //Map Hashed Concepts -> Concept
    {
      count n = in.HashedMappings.n();
      data >> "static const int Map_Concept_Size = " << n << ";";
      data >> "static const Map_Concept Map_Concept_Lookup[" <<
        "Map_Concept_Size] = {";

      Array<UUIDv4> HashedMappingsKeys = in.HashedMappings.Keys();
      Array<UUIDv4> HashedMappingsValues = in.HashedMappings.Values();
      for(count i = 0; i < HashedMappingsKeys.n(); i++)
        data >>
          "/*MC" << i << "*/ {{" <<
          ToString(HashedMappingsKeys[i]) << "}, {" <<
          ToString(HashedMappingsValues[i]) << "}}" <<
          ((i + 1 < n) ? "," : "");
      data >> "};";
    }
    data >> "}";

    //Identifiers
    String identifiers_data;
    {
      Array<String> IdentifierToConceptKeys = in.IdentifierToConcept.Keys();
      for(count i = 0; i < IdentifierToConceptKeys.n(); i++)
      {
        identifiers_data >> "  extern const UUIDv4 " <<
          ToString(in, IdentifierToConceptKeys[i]) << ";";
      }
    }

    //Insert the data into the templates.
    String mica_h = TemplateIn["mica-en-cpp.h"];
    mica_h.Replace("@@@@@1@@@@@", identifiers_data);
    mica_h.Replace("@@@@@2@@@@@", data1);
    mica_h.Replace("@@@@@3@@@@@", data);
    TemplateOut["include/mica.h"] = mica_h;
  }
};

////////////////////////////////////////////////////////////////////////////////

class Port_en_js : public PortBase
{
  String ToString(PRIM_NAMESPACE::UUIDv4 u)
  {
    String s = u;
    s.Replace("-", "");
    return s;
  }

  String ToString(Interpreter& in, PRIM_NAMESPACE::String s)
  {
    return LocalizedString(s, in.Locale).Text();
  }

  public:

  virtual ~Port_en_js();

  void Create(Interpreter& in, Tree<String>& TemplateIn,
    Tree<String>& TemplateOut)
  {
    String data;
    String data_namespace;

    //Definitions of identifiers
    {
      Array<String> IdentifierToConceptKeys = in.IdentifierToConcept.Keys();
      Array<Pointer<Interpreter::Concept> > IdentifierToConceptValues =
        in.IdentifierToConcept.Values();
      for(count i = 0; i < IdentifierToConceptKeys.n(); i++)
      {
        data >> "mica." <<
          ToString(in, IdentifierToConceptKeys[i]) << " = '" <<
          ToString(IdentifierToConceptValues[i]->ID) << "'";
        data_namespace >> ToString(in, IdentifierToConceptKeys[i]) <<
          " = mica." << ToString(in, IdentifierToConceptKeys[i]);
      }
      data++;
      data_namespace++;
    }

    //Key -> Concept
    {
      data >> "mica._key_concept = {";
      Array<String> DefinitionNameToConceptKeys =
        in.DefinitionNameToConcept.Keys();
      Array<Pointer<Interpreter::Concept> > DefinitionNameToConceptValues =
        in.DefinitionNameToConcept.Values();
      count n = DefinitionNameToConceptKeys.n();
      for(count i = 0; i < DefinitionNameToConceptKeys.n(); i++)
        data >>
          "/*KC" << i << "*/ \"" <<
          DefinitionNameToConceptKeys[i] << "\":'" <<
          ToString(DefinitionNameToConceptValues[i]->ID) << "'" <<
          ((i + 1 < n) ? "," : "");
      data >> "}";
      data++;
    }

    //Concept -> Length
    {
      count n = in.IDToConcept.n();
      data >> "mica._concept_length = {";
      Array<UUIDv4> IDToConceptKeys = in.IDToConcept.Keys();
      Array<Pointer<Interpreter::Concept> > IDToConceptValues =
        in.IDToConcept.Values();
      for(count i = 0; i < IDToConceptKeys.n(); i++)
        data >>
          "/*CL" << i << "*/ '" <<
          ToString(IDToConceptKeys[i]) << "':" <<
          (IDToConceptValues[i]->Cyclic ? "-" : "") <<
          IDToConceptValues[i]->Elements.n() <<
          ((i + 1 < n) ? "," : "");
      data >> "}";
      data++;
    }

    //{Sequence, Concept} -> Index
    {
      count n = in.SequenceConceptToIndex.n();
      data >> "mica._sequenceConcept_index = {";
      Array<Interpreter::SequenceConcept> SequenceConceptKeys;
      SequenceConceptKeys = in.SequenceConceptToIndex.Keys();
      Array<integer> SequenceConceptValues;
      SequenceConceptValues = in.SequenceConceptToIndex.Values();
      for(count i = 0; i < SequenceConceptKeys.n(); i++)
        data >>
          "/*SCI" << i << "*/ '" <<
          ToString(SequenceConceptKeys[i].SequenceItem) << "|"
          << ToString(SequenceConceptKeys[i].ConceptItem) << "':"
          << SequenceConceptValues[i] <<
          ((i + 1 < n) ? "," : "");
      data >> "}";
      data++;
    }

    //{Sequence, Index} -> Concept
    {
      count n = in.SequenceIndexToConcept.n();
      data >> "mica._sequenceIndex_concept = {";
      Array<Interpreter::SequenceIndex> SequenceIndexKeys;
      SequenceIndexKeys = in.SequenceIndexToConcept.Keys();
      Array<UUIDv4> SequenceIndexValues;
      SequenceIndexValues = in.SequenceIndexToConcept.Values();
      for(count i = 0; i < SequenceIndexKeys.n(); i++)
        data >>
          "/*SIC" << i << "*/ '" <<
          ToString(SequenceIndexKeys[i].SequenceItem) << "|"
          << SequenceIndexKeys[i].Index << "':'"
          << ToString(SequenceIndexValues[i]) << "'" <<
          ((i + 1 < n) ? "," : "");
      data >> "}";
      data++;
    }

    //{Concept, Language} -> Key
    {
      count n = in.ConceptLanguageToKey.n();
      data >> "mica._conceptLanguage_key = {";
      Array<Interpreter::ConceptLanguage> ConceptLanguageKeys =
        in.ConceptLanguageToKey.Keys();
      Array<String> ConceptLanguageValues =
        in.ConceptLanguageToKey.Values();
      for(count i = 0; i < ConceptLanguageKeys.n(); i++)
        data >>
          "/*CLK" << i << "*/ '" <<
          ToString(ConceptLanguageKeys[i].ConceptItem) << "|"
          << ConceptLanguageKeys[i].Language << "':\"" <<
          ConceptLanguageValues[i] << "\"" <<
          ((i + 1 < n) ? "," : "");
      data >> "}";
      data++;
    }

    //Map Hashed Concepts -> Concept
    {
      count n = in.HashedMappings.n();
      data >> "mica._map_concept = {";
      Array<UUIDv4> HashedMappingsKeys = in.HashedMappings.Keys();
      Array<UUIDv4> HashedMappingsValues = in.HashedMappings.Values();
      for(count i = 0; i < HashedMappingsKeys.n(); i++)
        data >>
          "/*MC" << i << "*/ '" <<
          ToString(HashedMappingsKeys[i]) << "':'" <<
          ToString(HashedMappingsValues[i]) << "'" <<
          ((i + 1 < n) ? "," : "");
      data >> "}";
      data++;
    }

    //Insert the data into the templates.
    String data_js = TemplateIn["mica-en-js-data.js"];
    data_js.Replace("@@@@@1@@@@@", data);
    String data_namespace_js = TemplateIn["mica-en-js-global.js"];
    data_namespace_js.Replace("@@@@@1@@@@@", data_namespace);

    //Just copy the non-data portions.
    TemplateOut["mica.js"] = TemplateIn["mica-en-js.js"];
    TemplateOut["mica-data.js"] = data_js;
    TemplateOut["mica-global.js"] = data_namespace_js;
  }
};

class Port
{
  public:

  virtual ~Port();

  static Array<String> GetPortTemplates()
  {
    Array<String> TemplateFiles;
    if(!Directory::Files("templates", TemplateFiles))
    {
      if(!Directory::Files("../templates", TemplateFiles))
      {
        C::Out() >> "Could not locate templates directory";
        return Array<String>();
      }
    }
    return TemplateFiles;
  }

  static Array<String> GetPortFiles(Array<String> TemplateFiles, String Prefix)
  {
    Array<String> PortFiles;
    for(count i = 0; i < TemplateFiles.n(); i++)
      if(TemplateFiles[i].ToFilename().StartsWith(Prefix))
        PortFiles.Add() = TemplateFiles[i];
    return PortFiles;
  }

  static Tree<String> GetPortDataFromFile(Array<String> TemplateFiles)
  {
    Tree<String> t;
    for(count i = 0; i < TemplateFiles.n(); i++)
    {
      String s;
      File::Read(TemplateFiles[i], s);
      t[TemplateFiles[i].ToFilename()] = s;
    }
    return t;
  }

  static void Make(Interpreter& i, String PortPrefix,
    Pointer<PortBase> Porter)
  {
    Tree<String> In = GetPortDataFromFile(
      GetPortFiles(GetPortTemplates(), PortPrefix));
    Tree<String> Out;
    C::Out() >> "Porting " << PortPrefix << "...";
    Porter->Create(i, In, Out);
    Array<String> OutKeys = Out.Keys();
    for(count j = 0; j < OutKeys.n(); j++)
    {
      String Key = OutKeys[j];
      String Filename = Key;
      C::Out() >> " * Writing " << Filename;
      if(!File::Write(Filename, Out[Key]))
      {
        C::Out() >> "Failed to write: " << Filename;
        return;
      }
    }
  }
};

class Port_wiki : public PortBase
{
protected:
  String Primary;
  String Secondary;

  Tree<String> Strings;

public:
  virtual ~Port_wiki();

  Port_wiki()
  {
    Strings["en"] = "English";
    Strings["en-GB"] = "English (U.K.)";

    Primary = ""; Secondary = "en:";
    Strings["Category"] = "Category";
    Strings["Name"] = "Name";
    Strings["UUID"] = "UUID";
    Strings["Sequence Length"] = "Sequence Length";
    Strings["Cyclic"] = "Cyclic";
    Strings["Yes"] = "Yes";
    Strings["No"] = "No";
    Strings["Identifiers"] = "Identifiers";
    Strings["IdentifiersText"] = "The following programming language "
      "identifiers are associated with this concept.";
    Strings["Sequence"] = "Sequence";
    Strings["SequenceText"] =
      "This concept contains an ordered sequence of elements.";
    Strings["Mappings"] = "Mappings";
    Strings["MappingsText"] =
      "This concept is related to other concepts by the following mappings.";
    Strings["MappingsTooMany1"] =
      "This concept is related to other concepts by ";
    Strings["MappingsTooMany2"] = " mappings. Since there are so many, the "
      "mappings have been hidden on this page.";
    Strings["MainPageRedirect"] = "MediaWiki:Mainpage";
    Strings["MainPage"] = "Main Page";
    Strings["SpecialCategories"] = "Special:Categories";
    Strings["Languages"] = "Languages";
    Strings["LanguagesText"] =
      "This concept has other names in these languages.";
  }

  String ToString(PRIM_NAMESPACE::UUIDv4 u)
  {
    String s = u;
    s.Replace("-", "");
    s.Insert("U, 0x", 16);
    s.Prepend("0x");
    s.Append("U");
    return s;
  }

  String GetKeyByLanguage(Tree<String>& t)
  {
    Array<String> tKeys = t.Keys();
    for(count i = 0; i < tKeys.n(); i++)
    {
      String k = tKeys[i];
      if(Primary && k.StartsWith(Primary))
        return k;
      if(Secondary && k.StartsWith(Secondary))
        return k;
    }
    return "";
  }

  String Localized(String s)
  {
    if(Primary) s.EraseBeginning(Primary);
    if(Secondary) s.EraseBeginning(Secondary);
    return s;
  }

  String ToString(Interpreter& in, PRIM_NAMESPACE::String s)
  {
    return LocalizedString(s, in.Locale).Text();
  }

  String ToLang(Interpreter& in, PRIM_NAMESPACE::String s)
  {
    return LocalizedString(s, in.Locale).Language();
  }

  String GetCategoryPage(Interpreter& in, Pointer<Interpreter::Category> c)
  {
    in.Concepts.n(); //

    String mw;
    String CategoryName = Localized(GetKeyByLanguage(c->Definitions));
    String CategoryText = Localized(c->Definitions[
      GetKeyByLanguage(c->Definitions)]);
    if(!CategoryText)
      CategoryText = CategoryName;

    mw >> "  <page>";
    mw >> "    <title>" << Strings["Category"] << ":" <<
      CategoryName << "</title>";
    mw >> "    <revision>";
    mw >> "      <text xml:space=\"preserve\">" << CategoryText << "</text>";
    mw >> "    </revision>";
    mw >> "  </page>";
    return mw;
  }

  String GetConceptPage(Interpreter& in, Pointer<Interpreter::Concept> c)
  {
    in.Concepts.n(); //

    String mw;
    String NameString = Localized(GetKeyByLanguage(c->Definitions));
    String UUIDv4String = String(c->ID);
    String DefinitionString = Localized(c->Definitions[
      GetKeyByLanguage(c->Definitions)]);
    String CategoryName = Localized(GetKeyByLanguage(c->Parent->Definitions));

    String WikiText;

    WikiText >> "[[" << Strings["Category"] << ":" << CategoryName << "]]";
    WikiText >> "{| class=\"wikitable\"";
    WikiText >> "!|" << Strings["Name"] >> "|" << NameString;
    WikiText >> "|-";
    WikiText >> "!|" << Strings["UUID"] >> "|" << UUIDv4String;
    WikiText >> "|-";

    String ImageHostname;
    String ImageName = ImageHostname;
    ImageName << NameString << ".jpg";
    ImageName.Replace(" ", "_");
    WikiText >> "!|" << Strings["Image"] >> "|" << ImageName;
    WikiText >> "|-";

    if(c->Elements.n())
    {
      WikiText >> "|-";
      WikiText >> "!|" << Strings["Sequence Length"] >> "|" << c->Elements.n();
      if(c->Cyclic)
      {
        WikiText >> "|-";
        WikiText >> "!|" << Strings["Cyclic"] >> "|" <<
          (c->Cyclic ? Strings["Yes"] : Strings["No"]);
      }
    }
    WikiText >> "|}";
    WikiText >> DefinitionString;

    WikiText >> "==" << Strings["Identifiers"] << "==";
    WikiText >> Strings["IdentifiersText"];
    Array<String> IdentifierKeys = c->Identifiers.Keys();
    for(count i = 0; i < IdentifierKeys.n(); i++)
    {
      WikiText >> "* " << IdentifierKeys[i];
    }

    if(c->Elements.n())
    {
      WikiText >> "==" << Strings["Sequence"] << "==";
      WikiText >> Strings["SequenceText"];
      WikiText++;
      WikiText++;
      Array<UUIDv4> m = c->Elements;
      for(count j = 0; j < m.n(); j++)
      {
        if(j != 0)
          WikiText << ", ";
        UUIDv4 u = m[j];
        if(!UUIDv4Hash::IsUUIDv4Number(u))
        {
          String s = Localized(GetKeyByLanguage(
            in.IDToConcept[u]->Definitions));
          WikiText << "[[" << s << "]]";
        }
        else
          WikiText << "" << UUIDv4Hash::UUIDv4ToNumber(u);
      }
    }
    if(c->Mappings.n())
    {
      WikiText >> "==" << Strings["Mappings"] << "==";
      if(c->Mappings.n() > 1000)
      {
       WikiText >> Strings["MappingsTooMany1"] << c->Mappings.n() <<
         Strings["MappingsTooMany2"];
      }
      else
      {
        WikiText >> Strings["MappingsText"];
      }
    }
    for(count i = 0; i < c->Mappings.n() && c->Mappings.n() < 1000; i++)
    {
      WikiText >> "*";
      Array<UUIDv4> m = c->Mappings[i];
      for(count j = 0; j < m.n(); j++)
      {
        if(j == m.n() - 1)
        {
          WikiText << " ";
          unicode RightArrow = 0x2192;
          WikiText.Append(RightArrow);
        }
        else if(j != 0)
        {
          WikiText << " +";
        }

        UUIDv4 u = m[j];
        if(!UUIDv4Hash::IsUUIDv4Number(u))
        {
          String s = Localized(GetKeyByLanguage(
            in.IDToConcept[u]->Definitions));
          WikiText << " [[" << s << "]]";
        }
        else
          WikiText << " " << UUIDv4Hash::UUIDv4ToNumber(u);
      }
    }

    if(c->Definitions.n() > 1)
    {
      WikiText >> "==" << Strings["Languages"] << "==";
      WikiText >> Strings["LanguagesText"];
      WikiText >> "{| class=\"wikitable\"";
      Array<String> DefinitionsKeys = c->Definitions.Keys();
      for(count i = 0; i < DefinitionsKeys.n(); i++)
      {
        String l = ToLang(in, DefinitionsKeys[i]);
        String n = ToString(in, DefinitionsKeys[i]);
        String l_ = l, n_ = n;
        l_.ToLower();
        n_.Replace(" ", "_");

        WikiText >> "!|" << Strings[l] >> "|" << "[http://localhost" <<
            " " << n << "]";
        WikiText >> "|-";
      }
      WikiText >> "|}";
    }

    mw >> "  <page>";
    mw >> "    <title>" << NameString << "</title>";
    mw >> "    <revision>";
    mw >> "      <text xml:space=\"preserve\">" << WikiText << "</text>";
    mw >> "    </revision>";
    mw >> "  </page>";

    return mw;
  }

  void Create(Interpreter& in, Tree<String>& TemplateIn,
    Tree<String>& TemplateOut)
  {
    TemplateIn.n();

    const count MaxXMLSize = 500000;
    const count MaxRecordsPerXML = 100;
    count j = 1;
    count k = 0;
    String mw = "<mediawiki>";
    mw >> "  <page><title>" << Strings["MainPageRedirect"] << "</title>";
    mw >> "    <revision><text>" << Strings["SpecialCategories"] <<
      "</text></revision></page>";
    mw >> "  <page><title>" << Strings["MainPage"] << "</title>";
    mw >> "    <revision><text></text></revision></page>";

    for(count i = 0; i < in.Categories.n(); i++, j++)
    {
      mw++;
      mw >> GetCategoryPage(in, in.Categories[i]);
      mw++;
    }

    for(count i = 0; i < in.Concepts.n(); i++, j++)
    {
      if(j >= MaxRecordsPerXML || mw.n() > MaxXMLSize)
      {
        //Finish writing XML file.
        mw >> "</mediawiki>"; mw++;
        String Filename;
        Filename << (k < 100 ? "0" : "") << (k < 10 ? "0" : "") << k <<
          ".xml";
        k++;
        TemplateOut[Filename] = mw;
        j = 0;
        mw = "<mediawiki>";
      }
      mw++;
      mw >> GetConceptPage(in, in.Concepts[i]);
      mw++;
    }

    {
      //Finish writing XML file.
      mw >> "</mediawiki>"; mw++;
      String Filename;
      Filename << (k < 100 ? "0" : "") << (k < 10 ? "0" : "") << k <<
        ".xml";
      k++;
      TemplateOut[Filename] = mw;
    }
  }
};

class Port_wiki_en : public Port_wiki
{
  public:
  virtual ~Port_wiki_en();

  Port_wiki_en()
  {
    Primary = ""; Secondary = "en:";
    Strings["Category"] = "Category";
    Strings["Name"] = "Name";
    Strings["UUID"] = "UUID";
    Strings["Image"] = "Image";
    Strings["Sequence Length"] = "Sequence Length";
    Strings["Cyclic"] = "Cyclic";
    Strings["Yes"] = "Yes";
    Strings["No"] = "No";
    Strings["Identifiers"] = "Identifiers";
    Strings["IdentifiersText"] = "The following programming language "
      "identifiers are associated with this concept.";
    Strings["Sequence"] = "Sequence";
    Strings["SequenceText"] =
      "This concept contains an ordered sequence of elements.";
    Strings["Mappings"] = "Mappings";
    Strings["MappingsText"] =
      "This concept is related to other concepts by the following mappings.";
    Strings["MappingsTooMany1"] =
      "This concept is related to other concepts by ";
    Strings["MappingsTooMany2"] = " mappings. Since there are so many, the "
      "mappings have been hidden on this page.";
    Strings["MainPageRedirect"] = "MediaWiki:Mainpage";
    Strings["MainPage"] = "Main Page";
    Strings["SpecialCategories"] = "Special:Categories";
    Strings["Languages"] = "Languages";
    Strings["LanguagesText"] =
      "This concept has other names in these languages.";
  }
};

class Port_wiki_en_GB : public Port_wiki
{
  public:
  virtual ~Port_wiki_en_GB();

  Port_wiki_en_GB()
  {
    Primary = "en-GB:"; Secondary = "en:";
    Strings["Category"] = "Category";
    Strings["Name"] = "Name";
    Strings["UUID"] = "UUID";
    Strings["Image"] = "Image";
    Strings["Sequence Length"] = "Sequence Length";
    Strings["Cyclic"] = "Cyclic";
    Strings["Yes"] = "Yes";
    Strings["No"] = "No";
    Strings["Identifiers"] = "Identifiers";
    Strings["IdentifiersText"] = "The following programming language "
      "identifiers are associated with this concept.";
    Strings["Sequence"] = "Sequence";
    Strings["SequenceText"] =
      "This concept contains an ordered sequence of elements.";
    Strings["Mappings"] = "Mappings";
    Strings["MappingsText"] =
      "This concept is related to other concepts by the following mappings.";
    Strings["MappingsTooMany1"] =
      "This concept is related to other concepts by ";
    Strings["MappingsTooMany2"] = " mappings. Since there are so many, the "
      "mappings have been hidden on this page.";
    Strings["MainPageRedirect"] = "MediaWiki:Mainpage";
    Strings["MainPage"] = "Main Page";
    Strings["SpecialCategories"] = "Special:Categories";
    Strings["Languages"] = "Languages";
    Strings["LanguagesText"] =
      "This concept has other names in these languages.";
  }
};

PortBase::~PortBase() {}
Port_wiki::~Port_wiki() {}
Port_wiki_en::~Port_wiki_en() {}
Port_wiki_en_GB::~Port_wiki_en_GB() {}
Port_en_cpp::~Port_en_cpp() {}
Port_en_java::~Port_en_java() {}
Port_en_js::~Port_en_js() {}

int main()
{
  AutoRelease<Console> ConsoleRelease;

  //Parse and interpret the data.
  Parser p;
  Interpreter i(p);
  if(!p.Import() || !i.Interpret(p))
    return 0;

  Port::Make(i, "mica-en-cpp", new Port_en_cpp);
#if 0
  Port::Make(i, "en-js", new Port_en_js);
  Port::Make(i, "en-java", new Port_en_java);
  Port::Make(i, "wiki/en", new Port_wiki_en);
  Port::Make(i, "wiki/en-GB", new Port_wiki_en_GB);
#endif
  return 0;
}
