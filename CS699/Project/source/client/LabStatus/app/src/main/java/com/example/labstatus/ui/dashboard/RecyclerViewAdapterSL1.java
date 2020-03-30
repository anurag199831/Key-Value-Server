package com.example.labstatus.ui.dashboard;

import android.content.Context;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.example.labstatus.R;

import java.util.ArrayList;


public class RecyclerViewAdapterSL1 extends RecyclerView.Adapter<RecyclerViewAdapterSL1.ViewHolder> {

    ArrayList mValues;
    Context mContext;
    protected ItemListener mListener;
    public boolean isClickable = true;

    public RecyclerViewAdapterSL1(Context context, ArrayList values, ItemListener itemListener) {

        mValues = values;
        mContext = context;
        mListener=itemListener;
    }

    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {

        public TextView textView;

        public RelativeLayout relativeLayout;
        DashboardViewModel item;

        public ViewHolder(View v) {

            super(v);

            v.setOnClickListener(this);
            textView = (TextView) v.findViewById(R.id.textView);
            relativeLayout = (RelativeLayout) v.findViewById(R.id.relativeLayout);

        }

        public void setData(DashboardViewModel item) {
            try {
                this.item = item;

                textView.setText(item.text);

                relativeLayout.setBackgroundColor(Color.parseColor(item.color));
                /*if (item.text=="" && relativeLayout.getVisibility() == View.VISIBLE) {
                    relativeLayout.setVisibility(View.GONE);
                }*/
            }
            catch(Exception e){
                e.printStackTrace();
            }

        }


        @Override
        public void onClick(View view) {
            if(!isClickable)
            {
                return;
            }
            if (mListener != null) {
                mListener.onItemClick(item);
            }
        }
    }

    @Override
    public RecyclerViewAdapterSL1.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {

        View view = LayoutInflater.from(mContext).inflate(R.layout.recycler_view_item, parent, false);

        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        try {
            DashboardViewModel temp = (DashboardViewModel) mValues.get(position);
            if (temp.text == "") {
                holder.itemView.setVisibility(View.GONE);
                holder.itemView.setLayoutParams(new RecyclerView.LayoutParams(0, 0));
            } else {
                holder.setData(temp);
                holder.itemView.setVisibility(View.VISIBLE);
                holder.itemView.setLayoutParams(new RecyclerView.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
            }
        }
        catch(Exception e){
            e.printStackTrace();
        }
    }


 /*   @Override
    public void onBindViewHolder(ViewHolder Vholder, int position) {
        Vholder.setData(mValues.get(position));

    }*/
  /*  @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        holder.setData(mValues.get(position));

    }
*/


    @Override
    public int getItemCount() {

        return mValues.size();
    }

    public interface ItemListener {
        void onItemClick(DashboardViewModel item);
    }
}
