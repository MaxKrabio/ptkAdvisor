#include "ptk.h"
#include "loadingtransport.h"
#include "tipper.h"
#include <xlsxwriterpp/worksheet.hpp>

namespace
{
using namespace::machine;
const size_t OUT_PARAMS_COUNT = 16;

std::vector<std::string> columnNames = { "L",
                                         "T",
                                         "Gamma",
                                         "название самосвала",
                                         "емкость ковша",
                                         "произоводительность погрузочной машины м3.час",
                                         "время обслуживания погрузочной машиной автосамосвала мин",
                                         "время движения груженного и порожнего автосамосвала в мин",
                                         "общее время рейса автосамосвала",
                                         "производительность автосамосвала м3.ч",
                                         "количество необходимых автосамосвалов",
                                         "время необходимое птк для выполнения 1000 м3 земляных работ час.тыс м3",
                                         "расход дизеля птк на 1000 м3",
                                         "расход электроэнергии эл. экскаватором на 1000 квт х час.тыс м3",
                                         "затраты труда на 1000 м3 чел/cмены",
                                         "производителность ПТК за смену м3.см",
                                         "производителность 1 рабочего за смену м3.см",
                                         "затраты на гражданское хозяйство на 1 маш.-час(руб.маш-час)",
                                         "стоимость часа эксплуатации всех машин птк руб.час",
                                         "стоимость 1000 m3 эксплуатации всех машин птк тыс.руб.час/м3",
                                         "время, необходимое для выполнения птком объема земляных работ А, смены",
                                         "энергоемкость на 1 м3 земляных работ квт час на м3",
                                         "энергоемкость на 1 м3 земляных работ (л.с)"
                                       };
std::vector<double> calculateExportComplex(int workWay,
                                           double Gamma,
                                           std::shared_ptr<machine::Tipper> tipper,
                                           std::shared_ptr<machine::LoadingTransport> loadingTransport)
{
    std::vector<double> outParams;

    outParams.resize(OUT_PARAMS_COUNT);

    outParams[Qp] = 60 * loadingTransport->getParam(machine::loadingT::E) *
            loadingTransport->getParam(loadingT::CyclesPerMin) *
            loadingTransport->getParam(loadingT::Ke) *
            loadingTransport->getParam(loadingT::Kv);

    outParams[to] = loadingTransport->getParam(loadingT::tn) +
            loadingTransport->getParam(loadingT::tz);

    outParams[tdv] = 2 * workWay * 60 / tipper->getParam(tipperT::Vpc);

    outParams[tob] = outParams[to] + outParams[tdv] + loadingTransport->getParam(loadingT::tp);

    outParams[Qa] = (60 * tipper->getParam(tipperT::qa)) *
            tipper->getParam(tipperT::Kq) / (Gamma * outParams[tob]);

    outParams[Na] = (outParams[Qp] / outParams[Qa]) + 1;

    outParams[tk] = 1000 / outParams[Qp];

    outParams[DiselPer1000] = (loadingTransport->getParam(loadingT::Dp) +
                               tipper->getParam(tipperT::Da) *
                               outParams[Na]) * outParams[tk] / 1000;

    outParams[EPer1000] = loadingTransport->getParam(loadingT::Ee) * outParams[tk];

    outParams[Qptk] = outParams[Qp] * T;

    outParams[WorkPer1000] = loadingTransport->getParam(loadingT::N) *
            outParams[tk] / T;

    outParams[Qr] = outParams[Qp] * T / loadingTransport->getParam(loadingT::N);

    outParams[Cg] = tipper->getParam(tipperT::Ckg) *
            tipper->getParam(tipperT::Kta) * 1000;

    outParams[Cptk] = loadingTransport->getParam(loadingT::Cp) +
            (tipper->getParam(tipperT::Ca) +
             outParams[Cg]) * outParams[Na];
    outParams[Toob] = loadingTransport->getParam(loadingT::A) / outParams[Qptk];

    outParams[kvtPTK1] = (loadingTransport->getParam(loadingT::Wp) +
                          outParams[Na]+ tipper->getParam(tipperT::Wa)) / outParams[Qp];

    outParams[lsPTK1] = outParams[kvtPTK1] / 0.7355;

    return outParams;
}

bool PrepareExportToExel(xlsxwriter::Workbook& wb, const std::string& path, const std::vector<double>& params, size_t& index)
{

    xlsxwriter::Worksheet ws = wb.get_worksheet_by_name("sheet");


    //        ws.cell(textPosition + std::to_string(index++)).value("L=");
    //        ws.cell(textPosition + std::to_string(index++)).value("T=");
    //        ws.cell(textPosition + std::to_string(index)).value("Gamma=");

    //        sindex = index;

    for (auto param: params)
    {
        //            std::string pos = textPosition + std::to_string(index);
        ws.write_number(2, index, param);
        ++index;
    }
    /*try {
        wb.save(filename);

    } catch (const std::exception& ex) {
        std::cerr << ex.what();
    }*/

    return true;
}

}

machine::PTK::PTK(std::vector<int>&& _earthTransportingLengths, MachineComplex_map&& _mam):
    earthTransportingLengths(_earthTransportingLengths),
    machineComplex(_mam),
    params(3, std::rand())
{

}

void machine::PTK::ExportToXlsx(xlsxwriter::Workbook& wb, const std::string& path, ExportComplex_vec &complexes, size_t verticalOffset)
{
    //    wb.load(path.data());
    xlsxwriter::Worksheet ws = wb.get_worksheet_by_name("sheet");

    xlsxwriter::Row startChar = 2;
    xlsxwriter::Column startOffset = verticalOffset;
    //check file existing. If file exist - write on the new ws. Either - write on the active ws.
    for (auto&& complex : complexes)
    {
        for (const double& paramValue : complex)
        {
            ws.write_number(startChar, startOffset, paramValue);
        }
        ++startChar;
        startOffset = verticalOffset;
    }
    //    ws.cell("C3").formula("=RAND()");
    //    ws.merge_cells("C3:C4");
    //    ws.freeze_panes("B2");
    //    wb.save(path.data());
}



bool machine::PTK::Export(std::string_view path)
{
    return true;
}

bool machine::PTK::Import(std::string_view path)
{
    return true;
}

IMachine *machine::PTK::Copy()
{

}

void machine::PTK::SetParam(machine::ptkT num, double value)
{
    params[num] = value;
}

void machine::PTK::Processing(xlsxwriter::Workbook& wb, const std::string& path)
{
    size_t cell_x = 1;
    for (auto workWay: earthTransportingLengths)
    {


        for (const auto& [tipper, loadingTransports]: machineComplex)
        {
            ExportComplex_vec exportComplex;
            exportComplex.resize(loadingTransports.size());
            for (const auto& loadingTransport: loadingTransports)
            {
                auto val = calculateExportComplex(workWay, params[Gamma],tipper, loadingTransport);
                exportComplex.emplace_back(val);
            }
            PrepareExportToExel(wb, path, params, cell_x);
            ExportToXlsx(wb,"", exportComplex, cell_x);
        }
        cell_x += 25;
    }
}

machine::PTK::~PTK()
{

}
