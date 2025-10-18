import argparse
import pathlib
#from xls_parser import parse_datapoints, Filter, dump_inputs, dump_sensors, translate, Datapoint
import openpyxl

from generate_presets import Preset, Filter, Datapoint, _translate

def wez_before_translate(datapoints: list[Datapoint], _: str):
    # Patch type of "Status vent. regulation" from U8 to LIST
    for dp in  datapoints:
        if dp.datapoint == 2051:
            dp.type_name = 'LIST'

def hv_before_translate(datapoints: list[Datapoint], _: str):
    # Patch type of "Status vent. regulation" from U8 to LIST
    for dp in  datapoints:
        if dp.datapoint == 39652:
            dp.type_name = 'LIST'

def wez_before_dump(datapoints: list[Datapoint], locale: str):
    rows = {
        1379, 1380, 1381, # Heating operation choice
        1382, 1384, 1386, # normal room temp.
        1383, 1385, 1387, # conservation romm temp.
        1414, 1415, 1416, # actual flow temperature
    }

    dp_list = [
        1,
        2, # Vorlauf
        1001,
        1002,
        1020,
        1021,
        2051,
        3050,
        3051,
        3053,
        1022, # eher Name als Index
        #1009,
        3032,
        7036,
        ]

    # add heat circle number
    for dp in datapoints:
        if dp.row in rows:
            dp.name = f'{dp.name} ({dp.function_number+1})'
        elif dp.datapoint in dp_list:
            dp.name = f'{dp.name} ({dp.function_number+1})'

    dp_list = [
        1009,
        2043
    ]
    for dp in datapoints:
        if dp.datapoint in dp_list:
            dp.name = f'{dp.name} {dp.function_name}'

def bd_before_dump(datapoints: list[Datapoint], locale: str):
    translations = {
        'en': {
            'BM_83_0_0_0': 'Room actual'
        },
        'de': {
            'BM_83_0_0_0': 'Raum-Ist'
        },
        'fr': {
            'BM_83_0_0_0': 'Valeur réelle pièce'
        },
        'it': {
            'BM_83_0_0_0': 'Ambiente-effettivo'
        }
    }
    _translate(datapoints, locale, translations)
    
if __name__ == "__main__":
    presets = [
        # Preset('WEZ1', Filter(rows=[
        #     1378, # AF1 - outdoor sensor 1
        #     1379, 1380, 1381, # Heating operation choice
        #     1382, 1384, 1386, # normal room temp.
        #     1383, 1385, 1387, # conservation romm temp.
        #     1414, 1415, 1416, # actual flow temperature
        #     1397, # hot water operation choice
        #     1398, # Normal hot water temp.
        #     1399, # Conservation hot water temp.
        #     1400, # Hot water setpoint
        #     1401, # hot water temp.
        #     1437, # WEZ output
        #     24778, # Electrical energy WEZ MWh
        #     26649, # Heat quantity heating
        #     26653, # Heat quantity DHW
        # ]), before_dump=wez_before_dump),
        Preset('WEZ1', Filter(unit_names=["WEZ"], unit_ids=[1], register_addresses=[
            #1477, # AF1 Außenfühler
            #1520, # Sollwert für Heizkreisbetrieb
            #1510, 1511,# Raum-Ist
            #1521, # Sollwert für Speicherbetrieb
            #18725, # Wärmeerzeuger
            #1490, 1491, # Vorlauf Soll
            #1501, 1502, # Status Heizkreisregelung
            #1504, # Status Warmwasserregelung
            1477, # AF1 - Aussenfühler 1
            1520, # Sollwert für Heizkreisbetrieb
            1511, # Raum-Ist
            1521, # Sollwert für Speicherbetrieb
            1510, # Raum-Ist
            1515, # Vorlauf-Ist
            1514, # Vorlauf-Ist
            1513, # Vorlauf-Ist
            1500, # Warmwasser-Ist SF
            18725, # Wärmeerzeuger-Ist
            18742, # Rücklauftemperatur Wärmeerzeuger
            1522, # Leistungsbegrenzung
            1525, # WEZ-Temperatur
            1529, # Heizung Sollwert
            18740, # Drehzahl Hauptpumpe
            1530, # Speicher Sollwert
            1531, # WEZ Sollwert
            1534, # Fehlercode vom Automaten
            1535, # Rücklauftemperatur
            1536, # WEZ-Leistung
            1537, # Absolute Leistung
            1539, # WEZ-Status
            1540, # Betriebsstatus
            27490, # Coefficient of Performance
            27491, # Sondenvorlauf-/Ansaugtemp.
            27492, # Sondenrücklauf-/Verdampferoberfl.temp.
            18760, # Anlage Vorlauftemperatur Heizen
            19868, # Anlage Vorlauftemperatur WW
            1494, # Raum-Soll
            1493, # Raum-Soll
            19563, # Vorlauf-Soll
            19562, # Vorlauf-Soll
            1499, # Warmwasser-Soll
            18724, # Wärmeerzeuger-Soll
            18769, # Sollwert Leistung Wärmeerzeuger
            18745, # Sollwert Leistung Wärmeerzeuger
            18767, # Sollwert Leistung Wärmeerzeuger
            19706, # Pumpe
            19707, # Pumpe
            19659, # Mischer
            19658, # Mischer
            18739, # Pumpe Wärmeerzeuger
            18757, # Pumpe Wärmeerzeuger
            19754, # SLP Warmwasser-Ladepumpe
            18761, # Anlagetemp.Soll Heizen aktuell
            18762, # Anlagetemp.Soll WW aktuell
            27534, # Sammelstörung Störmeldeausgang
            18768, # 0 - 100% aktuelle Anf. an WE
            18770, # 0 - 100% aktuelle Anf. an WE
            1501, # Status Heizkreisregelung
            1502, # Status Heizkreisregelung
            1504, # Status Warmwasserregelung
            18722, # Status Wärmeerzeugerregelung
            19872, # Führungs-WEZ
            27548, # Führungs-WEZ 
            27510, # Regelstrategie
            27511, # Regelstrategie
            1479, # Betriebswahl Heizung
            1478, # Betriebswahl Heizung
            1483, # Normal-Raumtemperatur Heizbetrieb
            1481, # Normal-Raumtemperatur Heizbetrieb
            1484, # Spar-Raumtemperatur Heizbetrieb
            1482, # Spar-Raumtemperatur Heizbetrieb
            1486, # Spar-Raumtemperatur Heizbetrieb
            1496, # Betriebswahl Warmwasser
            1497, # Normal-Warmwassertemperatur
            1498, # Spar-Warmwassertemperatur
            1491, # Vorlauf Soll Konstantanf. Heizen
            1490, # Vorlauf Soll Konstantanf. Heizen
            19482, # Vorlauf Soll Konstantanf. Kühlen
            1561, # Betriebswahl Wärmeerzeuger
            27550, # Max. Leistung Heiz-Betrieb
            27551, # Max. Leistung WW-Betrieb
            18738, # Wasserdruck
            18723, # FA-Status
            18726, # Modulation
            18737, # Betriebsmeldung
            27536, # Verfügbare el. Leistung gedämpft
            1546, # MK1 HW-Ausgang
            1547, # YK1+ HW-Ausgang
            1548, # YK1- HW-Ausgang
            1551, # VA1 HW-Ausgang
            1552, # VA2 HW-Ausgang
            27537, # Status Smart Grid
            #31741, # Status Smart Grid -> duplicate?
            27538, # Status Sollwert Erhöhung/Reduktion
            19049, # Info 1 - vermutlich AVF W (VE1)
            19050, # Info 2 - vermutlich AVF H (VE2)
            25611, # Aktuelle elektr. Leistungsaufnahme WEZ
            25612, # Aktuelle therm. Leistungsaufnahme WEZ
            27467, # JAZ Jahresarbeitszahl WEZ
            1564, # Emissionstest aktivieren
            27542, # SG Mindestleistung
            27545, # Smart Grid über Systembus
            27546, # Auslöser Smart Grid Funktion
        ]),
        before_dump=wez_before_dump,
        before_translate=wez_before_translate,
        #prefix="WEZ1"
        ),
        Preset('WEZ2', Filter(unit_names=["WEZ"], unit_ids=[2], register_addresses=[
            #1653, # Sollwert für Heizkreisbetrieb
            #27553, # Warmwasser-Ist SF2
            ##1378, # AF1 - outdoor sensor 1
            ##1379, 1380, 1381, # Heating operation choice
            ##1382, 1384, 1386, # normal room temp.
            ##1383, 1385, 1387, # conservation romm temp.
            ##1414, 1415, 1416, # actual flow temperature
            ##1397, # hot water operation choice
            ##1398, # Normal hot water temp.
            ##1399, # Conservation hot water temp.
            ##1400, # Hot water setpoint
            ##1401, # hot water temp.
            ##1437, # WEZ output
            ##24778, # Electrical energy WEZ MWh
            ##26649, # Heat quantity heating
            ##26653, # Heat quantity DHW
            #1632, 1633,
            #1637, # Status Warmwasserregelung
            1610, # AF1 - Aussenfühler 1
            1633, # Warmwasser-Ist SF
            27553, # Warmwasser-Ist SF2
            18786, # Wärmeerzeuger-Ist
            1655, # Leistungsbegrenzung
            18803, # Rücklauftemperatur Wärmeerzeuger
            1669, # WEZ-Leistung
            1670, # Absolute Leistung
            27560, # Coefficient of Performance
            1632, # Warmwasser-Soll
            18785, # Wärmeerzeuger-Soll
            19757, # SLP Warmwasser-Ladepumpe
            1637, # Status Warmwasserregelung
            18783, # Status Wärmeerzeugerregelung
            1629, # Betriebswahl Warmwasser
            1694, # Betriebswahl Wärmeerzeuger
            27620, # Max. Leistung Heiz-Betrieb
            18784, # FA-Status
            18787, # Modulation
            18798, # Betriebsmeldung
            1679, # MK1 HW-Ausgang
            1680, # YK1+ HW-Ausgang
            1681, # YK1- HW-Ausgang
            1683, # SLP HW-Ausgang
            1685, # VA2 HW-Ausgang
            27607, # Status Smart Grid
            19054, # Info 1 - vermutlich DF1-1 (21-015)
            19055, # Info 2 - vermutlich DF1-2 (21-018)
            25615, # Aktuelle elektr. Leistungsaufnahme WEZ
            25616, # Aktuelle therm. Leistungsaufnahme WEZ
            27468, # JAZ Jahresarbeitszahl WEZ
            1697, # Emissionstest aktivieren
        ]),
        before_dump=wez_before_dump,
        before_translate=wez_before_translate, 
        #prefix="WEZ2"
        ),
        ## filter the row number, not the datapoint :  based on UniName=HV, UnitId=520
        Preset('HV', Filter(rows=[ 
            22786, # Op. choice ventilation
            22787, # Normal ventilation modulation
            22788, # Eco ventilation modulation
            22789, # Ventilation modulation
            22790, # Humidity set value
            22791, # Humidity extract air
            # 22792, # VOC extract air # not relevant
            # 22793, # VOC outdoor air # not relevant
            # 22794, # Air quality control # not relevant
            22795, # Status vent regulation
            22796, # Outside air temp.            
            22797, # Extract air temp.
            22798, # Fan exhaust air set   
            # 23314, # Active error 1 # testing
            # 23323, # Active error 2
            # 23332, # Active error 3
            # 23341, # Active error 4
            # 23350, # Active error 5
            28099, # Maint.ctr.value message maint. (op. wks)
            # 28101, # Rem. run time maint. counter (op. weeks) # not relevant
            28110, # Cleaning count value message cleaning (operating weeks)
        ]), hv_before_translate),
        Preset('BM', [
            Datapoint(
                row=0,
                register_address=0,
                name='Room actual',
                unit_name='BM',
                unit_id=1,
                function_group=83,
                function_number=0,
                datapoint=0,
                type_name='S16',
                decimal=1,
                steps=1,
                function_name='',
                min=0,
                max=0,
                writable=False,
                unit='°C',
                text={},
                preset_id="BM"
            ),
        ], before_dump=bd_before_dump),
    ]

    parser = argparse.ArgumentParser(
        prog='Generate Presets',
        description='Generates sensors and inputs for Hoval devices',
    )

    #parser.add_argument('out_dir')
    #args = parser.parse_args()

    #out_dir = pathlib.Path(args.out_dir)
    out_dir = pathlib.Path("esphome/src/preset")
    
    path = pathlib.Path(__file__).parent.joinpath('TTE-GW-Modbus-datapoints_edited.xlsx')
    wb = openpyxl.load_workbook(filename=path, read_only=True)
    
    for preset in presets:
        preset.generate(wb, out_dir)